#pragma once
#include "Graphics.h"
#include <DirectXMath.h>
#include <array>
#include "d3dx12.h"
#include "GraphicsError.h"

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

		Microsoft::WRL::ComPtr<ID3D12Resource> pVertexBuffer;
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

			gfx.CommandList()->Close();

			ID3D12CommandList* commandLists[] = { gfx.CommandList().Get()};
			gfx.CommandQueue()->ExecuteCommandLists((UINT)std::size(commandLists), commandLists);

			gfx.Sync();
		}

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView{
			.BufferLocation = pVertexBuffer->GetGPUVirtualAddress(),
			.SizeInBytes = nVertices * sizeof(Vertex),
			.StrideInBytes = sizeof(Vertex),
		};
	}
	void Draw(Graphics& gfx)
	{
		
	}
};