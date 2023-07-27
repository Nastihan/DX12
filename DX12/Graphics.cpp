#include "Graphics.h"
#include "GraphicsError.h"
#include <d3d12.h>
#include <DirectXMath.h>
#include "d3dx12.h"

#include <ranges>
#include <stdexcept>
#include <cmath>
#include <numbers>

static float t = 0.f;
constexpr float step = 0.01f;

Graphics::Graphics(uint16_t width, uint16_t height, HWND hWnd)
	:
	width(width),height(height)
{
	constexpr UINT bufferCount = 2;

	// enable debug layer for d3d12
	Microsoft::WRL::ComPtr<ID3D12Debug> pDebugController;
	D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController)) >> chk;
	pDebugController->EnableDebugLayer();


	// dxgi factory
	Microsoft::WRL::ComPtr<IDXGIFactory4> pdxgiFactory;
	CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&pdxgiFactory)) >> chk;
	Microsoft::WRL::ComPtr<IDXGIAdapter> pAdapter;
	pdxgiFactory->EnumAdapters(1U, pAdapter.GetAddressOf());

	// device
	D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&pDevice)) >> chk;

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
			.Flags = 4,
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

void Graphics::DrawTriangle()
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
			const auto desc =CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices));

			pDevice->CreateCommittedResource(&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&pVertexBuffer)
			) >> chk;
		}

		// resource for cpu upload of the vertex data
		Microsoft::WRL::ComPtr<ID3D12Resource> pUploadVertexBuffer;
		{
			const CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_UPLOAD };
			const auto desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices));

			pDevice->CreateCommittedResource(&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&pUploadVertexBuffer)
			) >> chk;
		}

	}




}

void Graphics::BeginFrame()
{
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

		const FLOAT clearColor[] = {
							sin(2.f * t + 1.f) / 2.f + .5f,
							sin(3.f * t + 2.f) / 2.f + .5f,
							sin(5.f * t + 3.f) / 2.f + .5f,
							1.0f
		};
		// clear rtv
		const CD3DX12_CPU_DESCRIPTOR_HANDLE rtv{
					rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
					(INT)curBackBufferIndex, rtvDescriptorSize };

		pCommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
	}

}

void Graphics::EndFrame()
{
	// prepare buffer for presentation by transitioning to present state
	auto& backBuffer = pBackBuffers[curBackBufferIndex];
	const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	pCommandList->ResourceBarrier(1, &barrier);

	// submit command list
	pCommandList->Close() >> chk;
	ID3D12CommandList* const commandLists[] = { pCommandList.Get() };
	pCommandQueue->ExecuteCommandLists((UINT)std::size(commandLists), commandLists);

	pCommandQueue->Signal(pFence.Get(), fenceValue++) >> chk;

	// present
	pSwapChain->Present(1, 0);

	pFence->SetEventOnCompletion(fenceValue - 1, fenceEvent) >> chk;
	if (WaitForSingleObject(fenceEvent, INFINITE) == WAIT_FAILED) {
		GetLastError() >> chk;
	}

	if ((t += step) >= 2.f * std::numbers::pi_v<float>) {
		t = 0.f;
	}
}

void Graphics::QueueEmpty()
{
	pCommandQueue->Signal(pFence.Get(), fenceValue);
	pFence->SetEventOnCompletion(fenceValue, fenceEvent) >> chk;
	if (WaitForSingleObject(fenceEvent, 2000) == WAIT_FAILED) {
		GetLastError() >> chk;
	}
}
