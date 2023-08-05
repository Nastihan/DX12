#pragma once
#include "Graphics.h"
#include <DirectXMath.h>
#include <array>
#include <d3dcompiler.h>
#include "GraphicsError.h"
#include <DirectXMath.h>
#include <chrono>
#include <DxTex/include/DirectXTex.h>
#include <ranges>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "BindableInclude.h"

class AssimpTest
{
public:
	AssimpTest(Graphics& gfx)
	{
		Assimp::Importer imp;
		const auto pModel = imp.ReadFile("Models\\suzanne.obj", 
			aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);
		
		const auto pMesh = pModel->mMeshes[0];

		// Vertex data structure
		struct Vertex
		{
			DirectX::XMFLOAT3 pos;
			DirectX::XMFLOAT3 n;
		};
		// Vertex buffer stuff
		{
			std::vector<Vertex> vertices;
			vertices.reserve(pMesh->mNumVertices);
			for (unsigned int i = 0; i < pMesh->mNumVertices; i++)
			{
				vertices.push_back({
					{ pMesh->mVertices[i].x,pMesh->mVertices[i].y, pMesh->mVertices[i].z },
					{ *reinterpret_cast<DirectX::XMFLOAT3*>(&pMesh->mNormals[i]) }
					});
			}
			pVertexBuffer = std::make_unique<VertexBuffer>(gfx, vertices);
		}

		
		// Index buffer stuff
		{
			std::vector<WORD> indices;
			indices.reserve(pMesh->mNumFaces * 3);
			// index data

			for (unsigned int i = 0; i < pMesh->mNumFaces; i++)
			{
				const auto face = pMesh->mFaces[i];
				assert(face.mNumIndices == 3);
				indices.push_back(face.mIndices[0]);
				indices.push_back(face.mIndices[1]);
				indices.push_back(face.mIndices[2]);
			}

			pIndexBuffer = std::make_unique<IndexBuffer>(gfx, indices);
		}
		// cube texture
		{
			// load image data
			DirectX::ScratchImage image;
			DirectX::LoadFromWICFile(L"Models\\wood.jpg", DirectX::WIC_FLAGS_NONE, nullptr, image) >> chk;
			// generate mip chain
			DirectX::ScratchImage mipChain;
			DirectX::GenerateMipMaps(*image.GetImages(), DirectX::TEX_FILTER_BOX, 0, mipChain) >> chk;

			// texture resource
			{
				const auto& chainBase = *mipChain.GetImages();
				CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_DEFAULT };
				const auto resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
					chainBase.format,
					(UINT)chainBase.width,
					(UINT)chainBase.height
				);
				gfx.Device()->CreateCommittedResource(
					&heapProps,
					D3D12_HEAP_FLAG_NONE,
					&resourceDesc,
					D3D12_RESOURCE_STATE_COPY_DEST,
					nullptr,
					IID_PPV_ARGS(&pCubeTexture)) >> chk;
			}

			const auto temp = mipChain.GetImageCount();

			// collect subresource data 
			const auto subresourceData = std::ranges::views::iota(0, (int)mipChain.GetImageCount()) |
				std::ranges::views::transform([&](int i) {
				const auto img = mipChain.GetImage(i, 0, 0);
				return D3D12_SUBRESOURCE_DATA{
					.pData = img->pixels,
					.RowPitch = (LONG_PTR)img->rowPitch,
					.SlicePitch = (LONG_PTR)img->slicePitch,
				};
					}) |
				std::ranges::to<std::vector>();

					Microsoft::WRL::ComPtr<ID3D12Resource> pUploadBuffer;
					{
						const CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_UPLOAD };
						const auto uploadBufferSize = GetRequiredIntermediateSize(pCubeTexture.Get(), 0, (UINT)subresourceData.size());
						const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
						gfx.Device()->CreateCommittedResource(
							&heapProps,
							D3D12_HEAP_FLAG_NONE,
							&resourceDesc,
							D3D12_RESOURCE_STATE_GENERIC_READ,
							nullptr,
							IID_PPV_ARGS(&pUploadBuffer)
						) >> chk;
					}
					gfx.ResetCmd();
					// write commands to copy data to upload texture (copying each subresource) 
					UpdateSubresources(gfx.CommandList().Get(),
						pCubeTexture.Get(), pUploadBuffer.Get(),
						0, 0,
						(UINT)subresourceData.size(),
						subresourceData.data()
					);
					{
						// write command to transition texture to texture state  
						const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
							pCubeTexture.Get(),
							D3D12_RESOURCE_STATE_COPY_DEST,
							D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
						);
						gfx.CommandList()->ResourceBarrier(1, &barrier);
					}
					gfx.Execute();
					gfx.Sync();
		}
		// descriptor heap for the shader resource view
		{
			const D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
				.NumDescriptors = 1,
				.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
			};
			gfx.Device()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap)) >> chk;
		}
		// create handle to the srv heap and to the only view in the heap 
		srvHandle = (srvHeap->GetCPUDescriptorHandleForHeapStart());
		// create the descriptor in the heap 
		{
			const D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{
				.Format = pCubeTexture->GetDesc().Format,
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Texture2D{.MipLevels = pCubeTexture->GetDesc().MipLevels },
			};
			gfx.Device()->CreateShaderResourceView(pCubeTexture.Get(), &srvDesc, srvHandle);
		}

		// define root signature with a matrix of 16 32-bit floats used by the vertex shader (rotation matrix) 
		CD3DX12_ROOT_PARAMETER rootParameters[2]{};
		rootParameters[0].InitAsConstants(3 * (sizeof(DirectX::XMMATRIX) / 4), 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
		const CD3DX12_DESCRIPTOR_RANGE descRange{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV,1,0 };
		rootParameters[1].InitAsDescriptorTable(1, &descRange);

		// static sampler
		const CD3DX12_STATIC_SAMPLER_DESC sampler{ 0 };
		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
		// Allow input layout and vertex shader and deny unnecessary access to certain pipeline stages.
		const D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
		rootSignatureDesc.Init((UINT)std::size(rootParameters), rootParameters,
			1, &sampler, rootSignatureFlags);
		// serialize root signature
		Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
		if (const auto hr = D3D12SerializeRootSignature(
			&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			&signatureBlob, &errorBlob); FAILED(hr)) {
			if (errorBlob) {
				auto errorBufferPtr = static_cast<const char*>(errorBlob->GetBufferPointer());

				std::cout << " Error regarding Root Signature Serialization : " << errorBufferPtr << std::endl;
			}
			hr >> chk;
		}
		// Create root signature
		gfx.Device()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
			signatureBlob->GetBufferSize(), IID_PPV_ARGS(&pRootSignature)) >> chk;

		// create pso structure
		Graphics::PipelineStateStream pipelineStateStream;

		// define the vertex input layout
		const D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
			{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0 ,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		// load the VS & PS
		Microsoft::WRL::ComPtr<ID3DBlob> BlobVS;
		D3DReadFileToBlob(L"PhongVS.cso", &BlobVS) >> chk;
		Microsoft::WRL::ComPtr<ID3DBlob> BlobPS;
		D3DReadFileToBlob(L"PhongPS.cso", &BlobPS) >> chk;

		// filling pso structure
		pipelineStateStream.RootSignature = pRootSignature.Get();
		pipelineStateStream.InputLayout = { inputLayout,(UINT)std::size(inputLayout) };
		pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(BlobVS.Get());
		pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(BlobPS.Get());
		pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		pipelineStateStream.RTVFormats = {
			.RTFormats{ DXGI_FORMAT_R8G8B8A8_UNORM },
			.NumRenderTargets = 1,
		};

		// building pipeline state object
		D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
			sizeof(pipelineStateStream), &pipelineStateStream
		};
		gfx.Device()->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&pPipelineState)) >> chk;
	}
	void Draw(Graphics& gfx)
	{
		gfx.ResetCmd();
		gfx.CommandList()->SetPipelineState(pPipelineState.Get());
		gfx.CommandList()->SetGraphicsRootSignature(pRootSignature.Get());
		gfx.CommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		gfx.CommandList()->IASetVertexBuffers(0, 1, &pVertexBuffer->vertexBufferView);
		gfx.CommandList()->IASetIndexBuffer(&pIndexBuffer->indexBufferView);
		// bind the heap containing the texture descriptor 
		gfx.CommandList()->SetDescriptorHeaps(1, srvHeap.GetAddressOf());
		// bind the descriptor table containing the texture descriptor 
		gfx.CommandList()->SetGraphicsRootDescriptorTable(1, srvHeap->GetGPUDescriptorHandleForHeapStart());
		// transforms
		std::unique_ptr<TransformCbuf> transform = std::make_unique<TransformCbuf>(*this);
		auto mvp = transform->GetTransforms(gfx);
		gfx.CommandList()->SetGraphicsRoot32BitConstants(0, sizeof(mvp) / 4, &mvp, 0);
		gfx.ConfigForDraw();
		gfx.CommandList()->DrawIndexedInstanced(pIndexBuffer->nIndices, 1, 0, 0, 0);


		gfx.Execute();
		gfx.Sync();
	}
	DirectX::XMMATRIX GetTransform(Graphics& gfx)
	{
		auto updateRotationMatrix = []() -> DirectX::XMMATRIX
		{
			// Assuming rotationSpeed is the speed at which you want the triangle to rotate (in degrees per second)
			static float rotationSpeed = 23.0f; // Adjust this value to control rotation speed
			static float rotationAngle = 0.0f;
			static std::chrono::steady_clock::time_point prevTime = std::chrono::steady_clock::now();

			// Get the current time
			auto currentTime = std::chrono::steady_clock::now();

			// Calculate the time elapsed since the last frame
			float deltaTime = std::chrono::duration<float>(currentTime - prevTime).count();
			prevTime = currentTime;

			// Update the rotation angle
			rotationAngle += rotationSpeed * deltaTime;

			// Calculate the new rotation matrix
			DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(0.0f, 0.0f, 7.0f);
			DirectX::XMMATRIX rotationMatrix =
				DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(rotationAngle))
				* DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(rotationAngle))
				* DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(rotationAngle));

			return  rotationMatrix * translation;
		};
		const auto model = updateRotationMatrix();

		return model;
	}
private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> pRootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pPipelineState;

	// vertex buffer 
	std::unique_ptr<VertexBuffer> pVertexBuffer;
	// index buffer 
	std::unique_ptr<IndexBuffer> pIndexBuffer;
	// texture
	Microsoft::WRL::ComPtr<ID3D12Resource> pCubeTexture;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap;
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle;


};