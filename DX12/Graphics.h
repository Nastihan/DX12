#pragma once
#include <cstdint>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>

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
	uint64_t fenceValue = 0;
	HANDLE fenceEvent;
	// current back buffer index
	UINT curBackBufferIndex = 0;



	uint16_t width;
	uint16_t height;
};