#pragma once
#include <cstdint>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>

class Graphics
{
public:
	Graphics(uint16_t width, uint16_t height,HWND hWnd);
private:
	// DX objects
	Microsoft::WRL::ComPtr<ID3D12Device2> pDevice;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> pCommandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> pSwapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pCommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList6> pCommandList;


	uint16_t width;
	uint16_t height;
};