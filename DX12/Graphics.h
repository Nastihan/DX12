#pragma once
#include <cstdint>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "GraphicsError.h"
#include "d3dx12.h"


class Graphics
{
public:
	Graphics(uint16_t width, uint16_t height,HWND hWnd);
	void DrawTriangle();
	void BeginFrame();
	void EndFrame();
	void QueueEmpty();
public:
	// getter functions for the drawables
	Microsoft::WRL::ComPtr<ID3D12Device2> Device()
	{
		return pDevice;
	}
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue()
	{
		return pCommandQueue;
	}
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator()
	{
		return pCommandAllocator;
	}
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList6> CommandList()
	{
		return pCommandList;
	}
	// synchronization functions
	void ResetCmd()
	{
		pCommandAllocator->Reset() >> chk;
		pCommandList->Reset(pCommandAllocator.Get(), nullptr) >> chk;
	}
	void Sync()
	{
		pCommandQueue->Signal(pFence.Get(), ++fenceValue) >> chk;
		pFence->SetEventOnCompletion(fenceValue, fenceEvent) >> chk;
		if (WaitForSingleObject(fenceEvent, INFINITE) == WAIT_FAILED)
		{
			GetLastError() >> chk;
		}
	}
	void Execute()
	{
		pCommandList->Close() >> chk;

		ID3D12CommandList* commandLists[] = { pCommandList.Get() };
		pCommandQueue->ExecuteCommandLists((UINT)std::size(commandLists), commandLists);
	}
	// universal configs
	void ConfigForDraw()
	{
		// universal configs
		// configure RS
		pCommandList->RSSetViewports(1, &viewport);
		pCommandList->RSSetScissorRects(1, &scissorRect);
		// set primitive topology
		pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// bind render target
		pCommandList->OMSetRenderTargets(1, &rtv, TRUE, nullptr);
	}
	// static declartion of pso stream structure
	struct PipelineStateStream
	{
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE RootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
		CD3DX12_PIPELINE_STATE_STREAM_VS VS;
		CD3DX12_PIPELINE_STATE_STREAM_PS PS;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
	};

private:
	// DX objects
	Microsoft::WRL::ComPtr<ID3D12Device2> pDevice;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> pCommandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> pSwapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	// !!!!!! note !!!!!! manually writing the buffer count
	Microsoft::WRL::ComPtr<ID3D12Resource> pBackBuffers[2];
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pCommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList6> pCommandList;
	// fence
	Microsoft::WRL::ComPtr<ID3D12Fence1> pFence;
	// viewport & scissor rect
	CD3DX12_RECT scissorRect;
	CD3DX12_VIEWPORT viewport;
public:

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv;
private:

	uint64_t fenceValue = 0;
	HANDLE fenceEvent;
	// current back buffer index
	UINT curBackBufferIndex = 0;



	uint16_t width;
	uint16_t height;
};