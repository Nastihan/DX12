#pragma once
#include "Graphics.h"
#include <DirectXMath.h>
#include <array>
#include <d3dcompiler.h>
#include "d3dx12.h"
#include "GraphicsError.h"
#include <iostream>
#include <DirectXMath.h>
#include <chrono>


class Cube
{
public:
	Cube(Graphics& gfx)
	{
		// Vertex data structure
		struct Vertex
		{
			DirectX::XMFLOAT3 pos;
			DirectX::XMFLOAT3 color;
		};
		UINT nVertices;
		// Vertex buffer stuff
		{
			// vertex data
			const Vertex vertices[] = {
				{ {-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 0.0f} }, // 0 
				{ {-1.0f,  1.0f, -1.0f}, {0.0f, 1.0f, 0.0f} }, // 1 
				{ {1.0f,  1.0f, -1.0f}, {1.0f, 1.0f, 0.0f} }, // 2 
				{ {1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f} }, // 3 
				{ {-1.0f, -1.0f,  1.0f}, {0.0f, 0.0f, 1.0f} }, // 4 
				{ {-1.0f,  1.0f,  1.0f}, {0.0f, 1.0f, 1.0f} }, // 5 
				{ {1.0f,  1.0f,  1.0f}, {1.0f, 1.0f, 1.0f} }, // 6 
				{ {1.0f, -1.0f,  1.0f}, {1.0f, 0.0f, 1.0f} }  // 7 
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
				0, 1, 2, 0, 2, 3,
				4, 6, 5, 4, 7, 6,
				4, 5, 1, 4, 1, 0,
				3, 2, 6, 3, 6, 7,
				1, 5, 6, 1, 6, 2,
				4, 0, 3, 4, 3, 7,
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
				pUploadIndexBuffer->Unmap(0,nullptr);
			}
			gfx.ResetCmd();
			gfx.CommandList()->CopyResource(pIndexBuffer.Get(), pUploadIndexBuffer.Get());
			gfx.Execute();
			gfx.Sync();
		}
		// create index buffer view
		indexBufferView = {
			.BufferLocation = pIndexBuffer->GetGPUVirtualAddress(),
			.SizeInBytes = nIndices * (UINT)sizeof(WORD),
			.Format = DXGI_FORMAT_R16_UINT,
		};


		// define root signature with a matrix of 16 32-bit floats used by the vertex shader (rotation matrix) 
		CD3DX12_ROOT_PARAMETER rootParameters[1]{};
		rootParameters[0].InitAsConstants(sizeof(DirectX::XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init((UINT)std::size(rootParameters), rootParameters,
			0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
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
			signatureBlob->GetBufferSize(), IID_PPV_ARGS(&pRootSignature));

		// create pso structure
		Graphics::PipelineStateStream pipelineStateStream;

		// define the vertex input layout
		const D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
			{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0 ,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
			{"COLOR",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}
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
		auto mvp = DirectX::XMMatrixTranspose(GetTransform(gfx));
		gfx.CommandList()->SetGraphicsRoot32BitConstants(0, sizeof(mvp) / 4, &mvp, 0);
		gfx.ConfigForDraw();
		gfx.CommandList()->DrawIndexedInstanced(36, 1, 0, 0, 0);
		// temporary lambda
		auto updateRotationMatrix = []() -> DirectX::XMMATRIX
		{
			// Assuming rotationSpeed is the speed at which you want the triangle to rotate (in degrees per second)
			static float rotationSpeed = 20.0f; // Adjust this value to control rotation speed
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
				* DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(rotationAngle));

			return  rotationMatrix * translation;
		};
		const auto model = updateRotationMatrix();
		const auto MVP =  DirectX::XMMatrixTranspose(model * gfx.GetCamera() * gfx.GetProjection());
		gfx.CommandList()->SetGraphicsRoot32BitConstants(0, sizeof(MVP) / 4, &MVP, 0);
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


};