#pragma once
#include "Graphics.h"
#include <ranges>

class VertexBuffer
{
public:
	template <typename V>
	VertexBuffer(Graphics& gfx, const std::vector<V>& vertices)
	{
		UINT nVertices = (UINT)vertices.size();


		// commited resource for the vertex buffer on the gpu si
		{
			const CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_DEFAULT };
			const auto desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(V) * vertices.size());

			gfx.Device()->CreateCommittedResource(&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				// D3D12_RESOURCE_STATE_COPY_DEST d3d12 turns this param to COMMON for effectivity
				D3D12_RESOURCE_STATE_COMMON,
				nullptr,
				IID_PPV_ARGS(&pVertexBuffer)
			) >> chk;
		}

		// resource for cpu upload of the vertex data
		Microsoft::WRL::ComPtr<ID3D12Resource> pUploadVertexBuffer;
		{
			const CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_UPLOAD };
			const auto desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(V) * vertices.size());

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
			V* mappedVertexData = nullptr;
			pUploadVertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedVertexData)) >> chk;

			// Use std::ranges::copy with a range adaptor to convert the vector into a range
			std::ranges::copy(vertices | std::views::transform([](const V& v) { return v; }), mappedVertexData);

			pUploadVertexBuffer->Unmap(0, nullptr);
		}
		gfx.ResetCmd();
		gfx.CommandList()->CopyResource(pVertexBuffer.Get(), pUploadVertexBuffer.Get());
		gfx.Execute();
		gfx.Sync();
		// vertex buffer view
		vertexBufferView = {
			.BufferLocation = pVertexBuffer->GetGPUVirtualAddress(),
			.SizeInBytes = nVertices * (UINT)sizeof(V),
			.StrideInBytes = sizeof(V),
		};
	}
public:
	Microsoft::WRL::ComPtr<ID3D12Resource> pVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

};