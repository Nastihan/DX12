#pragma once
#include "Graphics.h"

template <typename C>
class ConstantBuffer
{
public:
	ConstantBuffer(Graphics& gfx, const C& consts)
	{
		// constant buffer
		const CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_DEFAULT };
		const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(consts));

		gfx.Device()->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&pBuffer)
		) >> chk;

		{
			const CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_UPLOAD };
			const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(consts));
			gfx.Device()->CreateCommittedResource(
				&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&pUploadBuffer)
			) >> chk;
		}

		void* pData = nullptr;
		pUploadBuffer->Map(0, nullptr, &pData) >> chk;
		memcpy(pData, &consts, sizeof(consts));
		pUploadBuffer->Unmap(0, nullptr);

		gfx.ResetCmd();
		gfx.CommandList()->CopyResource(pBuffer.Get(), pUploadBuffer.Get());
		gfx.Execute();
		gfx.Sync();

		// create handle to the cbv-srv heap and to theview in the heap 
		D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = gfx.GetHeap()->GetCPUDescriptorHandleForHeapStart();

		// create the descriptor in the heap 
		{
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.BufferLocation = pBuffer->GetGPUVirtualAddress();
			cbvDesc.SizeInBytes = (UINT)sizeof(consts);
			gfx.Device()->CreateConstantBufferView(&cbvDesc, cbvHandle);
		}
	}
	void Update(Graphics& gfx,const C& consts)
	{
		// Map the upload buffer and keep it mapped during the lifetime of the application
		void* pData = nullptr;
		pUploadBuffer->Map(0, nullptr, &pData) >> chk;
		auto dataCopy = consts;
		// Update the constant buffer data with the latest values
		memcpy(pData, &dataCopy, sizeof(consts));
		pUploadBuffer->Unmap(0, nullptr);

		// Copy the data from the upload buffer to the default heap (pLightCBuf)
		gfx.ResetCmd();
		gfx.CommandList()->CopyResource(pBuffer.Get(), pUploadBuffer.Get());
		gfx.Execute();
		gfx.Sync();
	}
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> pBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> pUploadBuffer;

};