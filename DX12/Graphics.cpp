#include "Graphics.h"
#include <d3d12.h>
#include <ranges>
#include <stdexcept>
#include <cmath>
#include <numbers>
#include <iostream>
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_glfw.h"


Graphics::Graphics(uint16_t width, uint16_t height, HWND hWnd)
	:
	width(width), height(height)
{

	constexpr UINT bufferCount = 2;
	scissorRect = CD3DX12_RECT{ 0, 0, LONG_MAX, LONG_MAX };
	viewport = CD3DX12_VIEWPORT{ 0.0f, 0.0f, float(width), float(height) };

#ifndef NDEBUG
	// enable debug layer for d3d12
	Microsoft::WRL::ComPtr<ID3D12Debug1> pDebugController;
	D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController)) >> chk;
	pDebugController->EnableDebugLayer();
	pDebugController->SetEnableGPUBasedValidation(true);
#endif 
	
	UINT factoryFlags = 0U;
	
#ifndef NDEBUG
	factoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif 

	// dxgi factory
	Microsoft::WRL::ComPtr<IDXGIFactory4> pdxgiFactory;
	CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&pdxgiFactory)) >> chk;
	Microsoft::WRL::ComPtr<IDXGIAdapter> pAdapter;
	pdxgiFactory->EnumAdapters(1U, pAdapter.GetAddressOf());
	DXGI_ADAPTER_DESC desc;
 	pAdapter->GetDesc(&desc);
	
	std::cout << "Device description: ";
	std::wcout << (std::wstring)desc.Description << std::endl;

	// device
	D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&pDevice)) >> chk;

	// check for ray tracing
	{
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {}; 
		pDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5)) >> chk; 
		if (options5.RaytracingTier < D3D12_RAYTRACING_TIER_1_0)
		{
			throw std::runtime_error("Raytracing not supported on device");
		}
		else
		{
			std::cout << "Raytracing is supported on the device" << std::endl;
		}
	}

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

	// rtv descriptors and buffer references
	{
		rtv = (rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		for (int i = 0; i < bufferCount; i++)
		{
			pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffers[i])) >> chk;
			pDevice->CreateRenderTargetView(pBackBuffers[i].Get(), nullptr, rtv);
			rtv.Offset(rtvDescriptorSize);
		}
	}

	// CbvSrvUav descriptor heap for imgui and drawables constant buffers
	{
		const D3D12_DESCRIPTOR_HEAP_DESC desc =
		{
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			.NumDescriptors = 7,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
		};
		pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&cbvsrvuavDescriptorHeap)) >> chk;
	}
	{
		const D3D12_DESCRIPTOR_HEAP_DESC desc =
		{
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			.NumDescriptors = 1,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
		};
		pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&imguiHeap)) >> chk;
	}
	// depth buffer
	{
		const CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_DEFAULT };
		const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT,
			width, height,
			1, 0, 1, 0,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
		);
		const D3D12_CLEAR_VALUE clearValue = {
				.Format = DXGI_FORMAT_D32_FLOAT,
				.DepthStencil = { 1.0f, 0 },
		};
		pDevice->CreateCommittedResource(&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearValue,
			IID_PPV_ARGS(&pDepthBuffer)
		) >> chk;
	}

	// dsv descriptor heap
	{
		const D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
			.NumDescriptors = 1,
		};
		pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&dsvDescriptorHeap)) >> chk;
	}

	// dsv and handle
	dsv = { dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart() };
	pDevice->CreateDepthStencilView(pDepthBuffer.Get(), nullptr, dsv);

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

	// init imgui dx12 impl
	ImGui_ImplDX12_Init(pDevice.Get(), bufferCount, DXGI_FORMAT_R8G8B8A8_UNORM,
		imguiHeap.Get(),
		imguiHeap->GetCPUDescriptorHandleForHeapStart(),
		imguiHeap->GetGPUDescriptorHandleForHeapStart()
	);
}

Graphics::~Graphics()
{
	ImGui_ImplDX12_Shutdown();
	// !!!!!! should fix the annoying bug
	light = nullptr;
}

void Graphics::BeginFrame()
{
	// imgui frame begin
	if (imguiEnabled)
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}
	// advance backbuffer
	curBackBufferIndex = pSwapChain->GetCurrentBackBufferIndex();
	// 
	const auto rtvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	auto& backBuffer = pBackBuffers[curBackBufferIndex];
	// reset command list and command allocator
	ResetCmd();

	// get rtv handle for the buffer used in this frame
	rtv = {
			rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			(INT)curBackBufferIndex, rtvDescriptorSize };
	// clear the render target
	{
		const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		pCommandList->ResourceBarrier(1, &barrier); 

		const FLOAT clearColor[] = { 0.015f, 0.0f, 0.078f,1.0f };
		// clear rtv
		pCommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
	}
	// clear the depth stencil
	pCommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	Execute();
	Sync();
}

void Graphics::EndFrame()
{
	ResetCmd();

	// imgui frame end
	if (imguiEnabled)
	{
		ImGui::Render();
		ConfigForDraw();
		ImguiConfig();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCommandList.Get());
	}

	// prepare buffer for presentation by transitioning to present state
	auto& backBuffer = pBackBuffers[curBackBufferIndex];
	const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	pCommandList->ResourceBarrier(1, &barrier);

	// submit command list
	Execute();

	pCommandQueue->Signal(pFence.Get(), ++fenceValue) >> chk;

	// present
	pSwapChain->Present(1, 0);

	pFence->SetEventOnCompletion(fenceValue , fenceEvent) >> chk;
	if (WaitForSingleObject(fenceEvent, INFINITE) == WAIT_FAILED) {
		GetLastError() >> chk;
	}
}

void Graphics::EndFrameRT(Microsoft::WRL::ComPtr <ID3D12Resource> output)
{
	ResetCmd();

	// imgui frame end
	
	// prepare buffer for presentation by transitioning to present state
	auto& backBuffer = pBackBuffers[curBackBufferIndex];
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
	pCommandList->ResourceBarrier(1, &barrier);

	pCommandList->CopyResource(backBuffer.Get(), output.Get());

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
	pCommandList->ResourceBarrier(1, &barrier);

	if (imguiEnabled)
	{
		ImGui::Render();
		ConfigForDraw();
		ImguiConfig();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCommandList.Get());
	}

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	pCommandList->ResourceBarrier(1, &barrier);

	// submit command list
	Execute();

	pCommandQueue->Signal(pFence.Get(), ++fenceValue) >> chk;

	// present
	pSwapChain->Present(1, 0);

	pFence->SetEventOnCompletion(fenceValue, fenceEvent) >> chk;
	if (WaitForSingleObject(fenceEvent, INFINITE) == WAIT_FAILED) {
		GetLastError() >> chk;
	}
}

void Graphics::QueueEmpty()
{
	pCommandQueue->Signal(pFence.Get(), ++fenceValue);
	pFence->SetEventOnCompletion(fenceValue, fenceEvent) >> chk;
	if (WaitForSingleObject(fenceEvent, 2000) == WAIT_FAILED) {
		GetLastError() >> chk;
	}
}

void Graphics::SetCamera(DirectX::FXMMATRIX cam) noexcept
{
	camera = cam;
}

DirectX::XMMATRIX Graphics::GetCamera() const noexcept
{
	return camera;
}

void Graphics::SetProjection(DirectX::FXMMATRIX proj) noexcept
{
	projection = proj;
}

DirectX::XMMATRIX Graphics::GetProjection() const noexcept
{
	return projection;
}

void Graphics::SetLight(PointLight& light) noexcept
{
	this->light = &light;
}

PointLight& Graphics::GetLight() const noexcept
{
	return *light;
}

uint16_t Graphics::GetWidth() const noexcept
{
	return width;
}

uint16_t Graphics::GetHeight() const noexcept
{
	return height;
}

void Graphics::EnableImgui() noexcept
{
	imguiEnabled = true;
}

void Graphics::DisableImgui() noexcept
{
	imguiEnabled = false;
}

bool Graphics::IsImguiEnabled() const noexcept
{
	return imguiEnabled;
}

void Graphics::EnableRT() noexcept
{
	rtEnabled = true;
}

void Graphics::DisableRT() noexcept
{
	rtEnabled = false;
}

bool Graphics::RTEnabled() const noexcept
{
	return rtEnabled;
}
