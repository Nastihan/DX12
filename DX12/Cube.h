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

class Cube
{
public:
	Cube(Graphics& gfx)
	{
		// Vertex data structure
		struct Vertex
		{
			DirectX::XMFLOAT3 pos;
			DirectX::XMFLOAT2 tc;
		};
		UINT nVertices;
		// Vertex buffer stuff
		{
			// vertex data
			const Vertex vertices[] = {
					// Front face
					{ { -1.0f, -1.0f, -1.0f }, { 0.0f, 1.0f } }, // Bottom-left (0)
					{ { 1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f } }, // Bottom-right (1)
					{ { -1.0f, 1.0f, -1.0f }, { 0.0f, 0.0f } }, // Top-left (2)
					{ { 1.0f, 1.0f, -1.0f }, { 1.0f, 0.0f } }, // Top-right (3)

					// Back face
					{ { -1.0f, -1.0f, 1.0f }, { 1.0f, 1.0f } },  // Bottom-left (4)
					{ { 1.0f, -1.0f, 1.0f }, { 0.0f, 1.0f } },  // Bottom-right (5)
					{ { -1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },  // Top-left (6)
					{ { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },  // Top-right (7)

					// Left face
					{ { -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f } }, // Bottom-left (8)
					{ { -1.0f, 1.0f, -1.0f }, { 1.0f, 0.0f } }, // Top-left (9)
					{ { -1.0f, -1.0f, 1.0f }, { 0.0f, 1.0f } }, // Bottom-right (10)
					{ { -1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } }, // Top-right (11)

					// Right face
					{ { 1.0f, -1.0f, -1.0f }, { 0.0f, 1.0f } }, // Bottom-left (12)
					{ { 1.0f, 1.0f, -1.0f }, { 0.0f, 0.0f } }, // Top-left (13)
					{ { 1.0f, -1.0f, 1.0f }, { 1.0f, 1.0f } }, // Bottom-right (14)
					{ { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } }, // Top-right (15)

					// Bottom face
					{ { -1.0f, -1.0f, -1.0f }, { 0.0f, 1.0f } }, // Bottom-left (20)
					{ { 1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f } }, // Bottom-right (21)
					{ { -1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f } }, // Top-left (22)
					{ { 1.0f, -1.0f, 1.0f }, { 1.0f, 0.0f } }, // Top-right (23)

					// Top face
					{ { -1.0f, 1.0f, -1.0f }, { 0.0f, 1.0f } }, // Bottom-left (16)
					{ { 1.0f, 1.0f, -1.0f }, { 1.0f, 1.0f } }, // Bottom-right (17)
					{ { -1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } }, // Top-left (18)
					{ { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } }, // Top-right (19)

	
			};

			nVertices = (UINT)std::size(vertices);

			// commited resource for the vertex buffer on the gpu side
			{
				const CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_DEFAULT };
				const auto desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices));

				gfx.Device()->CreateCommittedResource(&heapProps,
					D3D12_HEAP_FLAG_NONE,
					&desc,
					// D3D12_RESOURCE_STATE_COPY_DEST d3d12 turns this param to COMMON for effectivity
					D3D12_RESOURCE_STATE_COMMON,
					nullptr,
					IID_PPV_ARGS(&pVertexBuffer)
				) >> chk;


				// uncomment to not get the warning every frame
				/*auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(pVertexBuffer.Get(),
					D3D12_RESOURCE_STATE_COMMON,
					D3D12_RESOURCE_STATE_COPY_DEST
				);
				pCommandList->ResourceBarrier(1, &barrier);*/
			}

			// resource for cpu upload of the vertex data
			Microsoft::WRL::ComPtr<ID3D12Resource> pUploadVertexBuffer;
			{
				const CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_UPLOAD };
				const auto desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices));

				gfx.Device()->CreateCommittedResource(&heapProps,
					D3D12_HEAP_FLAG_NONE,
					&desc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(&pUploadVertexBuffer)
				) >> chk;
			}
			// copy the vertex data to uploadBuffer
			{
				Vertex* mappedVertexData = nullptr;
				pUploadVertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedVertexData)) >> chk;
				std::ranges::copy(vertices, mappedVertexData);
				pUploadVertexBuffer->Unmap(0, nullptr);
			}
			gfx.ResetCmd();
			gfx.CommandList()->CopyResource(pVertexBuffer.Get(), pUploadVertexBuffer.Get());
			gfx.Execute();
			gfx.Sync();
		}
		// vertex buffer view
		vertexBufferView = {
			.BufferLocation = pVertexBuffer->GetGPUVirtualAddress(),
			.SizeInBytes = nVertices * (UINT)sizeof(Vertex),
			.StrideInBytes = sizeof(Vertex),
		};
		// Index buffer stuff
		UINT nIndices;
		{
			// index data
			const WORD indices[] = {
				0,2, 1,    2,3,1,
				4,5, 7,    4,7,6,
				8,10, 9,  10,11,9,
				12,13,15, 12,15,14,
				16,17,18, 18,17,19,
				20,23,21, 20,22,23
			};
			nIndices = (UINT)std::size(indices);

			// create commited resource for index buffer
			{
				const CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_DEFAULT };
				const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(indices));
				gfx.Device()->CreateCommittedResource(&heapProps,
					D3D12_HEAP_FLAG_NONE,
					&resourceDesc,
					D3D12_RESOURCE_STATE_COMMON,
					nullptr,
					IID_PPV_ARGS(&pIndexBuffer)
				) >> chk;
			}
			// create commited resource for cpu upload of the vertex data
			Microsoft::WRL::ComPtr<ID3D12Resource> pUploadIndexBuffer;
			{
				const CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_UPLOAD };
				const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(indices));
				gfx.Device()->CreateCommittedResource(&heapProps,
					D3D12_HEAP_FLAG_NONE,
					&resourceDesc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(&pUploadIndexBuffer)
				) >> chk;
			}
			// copy
			{
				WORD* mappedIndexData = nullptr;
				pUploadIndexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedIndexData)) >> chk;
				std::ranges::copy(indices, mappedIndexData);
				pUploadIndexBuffer->Unmap(0, nullptr);
			}
			gfx.ResetCmd();
			gfx.CommandList()->CopyResource(pIndexBuffer.Get(), pUploadIndexBuffer.Get());
			gfx.Execute();
			gfx.Sync();
		}
		// index buffer view
		indexBufferView = {
			.BufferLocation = pIndexBuffer->GetGPUVirtualAddress(),
			.SizeInBytes = nIndices * (UINT)sizeof(WORD),
			.Format = DXGI_FORMAT_R16_UINT,
		};
		// cube texture
		{
			// load image data
			DirectX::ScratchImage image;
			DirectX::LoadFromWICFile(L"Model\\wood.jpg", DirectX::WIC_FLAGS_NONE, nullptr, image) >> chk;
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
		rootParameters[0].InitAsConstants(sizeof(DirectX::XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
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
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		// load the VS & PS
		Microsoft::WRL::ComPtr<ID3DBlob> BlobVS;
		D3DReadFileToBlob(L"VertexShader.cso", &BlobVS) >> chk;
		Microsoft::WRL::ComPtr<ID3DBlob> BlobPS;
		D3DReadFileToBlob(L"PixelShader.cso", &BlobPS) >> chk;

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
		gfx.CommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
		gfx.CommandList()->IASetIndexBuffer(&indexBufferView);
		// bind the heap containing the texture descriptor 
		gfx.CommandList()->SetDescriptorHeaps(1, srvHeap.GetAddressOf());
		// bind the descriptor table containing the texture descriptor 
		gfx.CommandList()->SetGraphicsRootDescriptorTable(1, srvHeap->GetGPUDescriptorHandleForHeapStart());
		auto mvp = DirectX::XMMatrixTranspose(GetTransform(gfx));
		gfx.CommandList()->SetGraphicsRoot32BitConstants(0, sizeof(mvp) / 4, &mvp, 0);
		gfx.ConfigForDraw();
		gfx.CommandList()->DrawIndexedInstanced(36, 1, 0, 0, 0);


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
			DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
			DirectX::XMMATRIX rotationMatrix = 
				DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(rotationAngle))
					* DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(rotationAngle))
						* DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(rotationAngle));

			return  rotationMatrix * translation;
		};
		const auto model = updateRotationMatrix();

		const auto MVP = model * gfx.GetCamera() * gfx.GetProjection();

		return MVP;
	}
private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> pRootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pPipelineState;

	// vertex buffer & vertex buffer view  // should be a member variable for use in IASetVertexBuffers 
	Microsoft::WRL::ComPtr<ID3D12Resource> pVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	// index buffer 
	Microsoft::WRL::ComPtr<ID3D12Resource> pIndexBuffer;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
	// texture
	Microsoft::WRL::ComPtr<ID3D12Resource> pCubeTexture;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap;
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle;

};