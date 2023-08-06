#include "IndexBuffer.h"

IndexBuffer::IndexBuffer(Graphics& gfx,const std::vector<WORD>& indices)
{

	nIndices = (UINT)std::size(indices);

	//indexCountPerInstance = nIndices;

	// create commited resource for index buffer
	{
		const CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_DEFAULT };
		const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(WORD) * indices.size());
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
		const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(WORD) * indices.size());
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

	// index buffer view
	indexBufferView = {
	.BufferLocation = pIndexBuffer->GetGPUVirtualAddress(),
	.SizeInBytes = nIndices * (UINT)sizeof(WORD),
	.Format = DXGI_FORMAT_R16_UINT,
	};
}

