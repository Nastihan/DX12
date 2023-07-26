#include "Graphics.h"
#include "GraphicsError.h"
#include <d3d12.h>
#include "d3dx12.h"
#include <stdexcept>

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

	// rtv descriptor heap
	{
		const D3D12_DESCRIPTOR_HEAP_DESC desc = {
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			.NumDescriptors = bufferCount,
		};
		pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rtvDescriptorHeap)) >> chk;
	}
	const auto rtvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// rtv descriptors and buffer refrences
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		for (int i = 0; i < bufferCount; i++)
		{
			pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffers[i])) >> chk;
			pDevice->CreateRenderTargetView(pBackBuffers[i].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(rtvDescriptorSize);
		}
	}

	// command allocater
	pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pCommandAllocator)) >> chk;

	// command list
	pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		pCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&pCommandList)) >> chk;
	pCommandList->Close();

	// fence 
	pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence)) >> chk;

	// fence signalling event 
	fenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
	if (!fenceEvent)
	{
		GetLastError() >> chk;
		throw std::runtime_error("failed to create fence event");
	}
}

void Graphics::BeginFrame()
{
	UINT curBackBufferIndex;
	// advance backbuffer
	curBackBufferIndex = pSwapChain->GetCurrentBackBufferIndex();
	// 
	const auto rtvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	auto& backBuffer = pBackBuffers[curBackBufferIndex];
	// reset command list and command allocator
	pCommandAllocator->Reset() >> chk;
	pCommandList->Reset(pCommandAllocator.Get(), nullptr) >> chk;

	// clear the render target
	{
		const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		pCommandList->ResourceBarrier(1, &barrier); 

		FLOAT clearColor[] = { 0.2f,0.4f,0.1f,1.0f };
		const CD3DX12_CPU_DESCRIPTOR_HANDLE rtv{
					rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
					(INT)curBackBufferIndex, rtvDescriptorSize };

		pCommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
	}

}

void Graphics::EndFrame()
{
}
