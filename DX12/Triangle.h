#pragma once
#include "Graphics.h"
#include <DirectXMath.h>
#include <array>
#include <d3dcompiler.h>
#include "d3dx12.h"
#include "GraphicsError.h"
#include <iostream>




class Triangle
{
public:
	Triangle(Graphics& gfx)
	{
		// Vertex data structure
		struct Vertex
		{
			DirectX::XMFLOAT3 pos;
			DirectX::XMFLOAT3 color;
		};

		UINT nVertices;
		{
			// vertex data
			const Vertex vertices[]{
					{ {  0.00f,  0.50f, 0.0f }, { 1.0f, 0.0f, 0.0f } }, // top 
					{ {  0.43f, -0.25f, 0.0f }, { 0.0f, 0.0f, 1.0f } }, // right 
					{ { -0.43f, -0.25f, 0.0f }, { 0.0f, 1.0f, 0.0f } }, // left 
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

			gfx.CommandAllocator()->Reset() >> chk;
			gfx.CommandList()->Reset(gfx.CommandAllocator().Get(), nullptr) >> chk;

			gfx.CommandList()->CopyResource(pVertexBuffer.Get(), pUploadVertexBuffer.Get());

			gfx.Execute();

			gfx.Sync();
		}

		vertexBufferView = {
			.BufferLocation = pVertexBuffer->GetGPUVirtualAddress(),
			.SizeInBytes = nVertices * (UINT)sizeof(Vertex),
			.StrideInBytes = sizeof(Vertex),
		};

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
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
		gfx.CommandAllocator()->Reset() >> chk;
		gfx.CommandList()->Reset(gfx.CommandAllocator().Get(),nullptr) >> chk;
		gfx.CommandList()->SetPipelineState(pPipelineState.Get());
		gfx.CommandList()->SetGraphicsRootSignature(pRootSignature.Get());

		gfx.CommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		gfx.CommandList()->IASetVertexBuffers(0,1,&vertexBufferView);

		// define scissor rect 
		const CD3DX12_RECT scissorRect{ 0, 0, LONG_MAX, LONG_MAX };

		// define viewport 
		const CD3DX12_VIEWPORT viewport{ 0.0f, 0.0f, float(1600), float(900) };



		gfx.CommandList()->RSSetViewports(1, &viewport);
		gfx.CommandList()->RSSetScissorRects(1, &scissorRect);

		gfx.CommandList()->OMSetRenderTargets(1, &gfx.rtv, TRUE, nullptr);


		gfx.CommandList()->DrawInstanced(3, 1, 0, 0);

		gfx.Execute();
		gfx.Sync();
	}
private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> pRootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pPipelineState;

	// vertex buffer & vertex buffer view  // should be a member variable for use in IASetVertexBuffers 
	Microsoft::WRL::ComPtr<ID3D12Resource> pVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;


};