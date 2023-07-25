#include "Graphics.h"
#include "GraphicsError.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include "d3dx12.h"

Graphics::Graphics(uint16_t width, uint16_t height, HWND hWnd)
	:
	width(width),height(height)
{
	constexpr UINT bufferCount = 2;

	// dxgi factory
	Microsoft::WRL::ComPtr<IDXGIFactory4> pdxgiFactory;
	CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&pdxgiFactory)) >> chk;
	Microsoft::WRL::ComPtr<IDXGIAdapter> pAdapter;
	pdxgiFactory->EnumAdapters(1U, pAdapter.GetAddressOf());

	// device
	D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&pDevice)) >> chk;

	// command queue
	D3D12_COMMAND_QUEUE_DESC cqDesc = {
		.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
		.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
		.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
		.NodeMask = 0,
	};
	pDevice->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&pCommandQueue)) >> chk;

	// swap chain
	Microsoft::WRL::ComPtr<IDXGISwapChain4> pSwapChain;
	{
		const DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {
			.Width = width,
			.Height = height,
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
			.Stereo = FALSE,
			.SampleDesc = {
				.Count = 1,
				.Quality = 0
			},
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = bufferCount,
			.Scaling = DXGI_SCALING_STRETCH,
			.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
			.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
			.Flags = 0,
		};
		Microsoft::WRL::ComPtr<IDXGISwapChain1> pSwapChain1;
		pdxgiFactory->CreateSwapChainForHwnd(
			pCommandQueue.Get(),
			hWnd,
			&swapChainDesc,
			nullptr,
			nullptr,
			&pSwapChain1) >> chk;
		pSwapChain1.As(&pSwapChain) >> chk;
	}

}
