#pragma once
#include <cstdint>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "GraphicsError.h"
#include "d3dx12.h"
#include <DirectXMath.h>
#include <optional>

class PointLight;

class Graphics
{
public:
	Graphics(uint16_t width, uint16_t height,HWND hWnd);
	Graphics(const Graphics&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	~Graphics();
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
		pCommandList->OMSetRenderTargets(1, &rtv, TRUE, &dsv);
	}
	void ImguiConfig()
	{
		ID3D12DescriptorHeap* descriptorHeaps[] = { imguiHeap.Get() };
		pCommandList->SetDescriptorHeaps(1, descriptorHeaps);
	}
	// CBV SRV UAV heap getter
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetHeap()
	{
		return cbvsrvuavDescriptorHeap;
	}

	// static declartion of pso stream structure
	struct PipelineStateStream
	{
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE RootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
		CD3DX12_PIPELINE_STATE_STREAM_VS VS;
		CD3DX12_PIPELINE_STATE_STREAM_PS PS;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
	};
	// getters and setter for view & projection matrices
	void SetCamera(DirectX::FXMMATRIX cam) noexcept;
	DirectX::XMMATRIX GetCamera() const noexcept;
	void SetProjection(DirectX::FXMMATRIX proj) noexcept;
	DirectX::XMMATRIX GetProjection() const noexcept;
	void SetLight(PointLight& light) noexcept;
	PointLight& GetLight() const noexcept;
	void EnableImgui() noexcept;
	void DisableImgui() noexcept;
	bool IsImguiEnabled() const noexcept;
private:
	bool imguiEnabled = true;
	DirectX::XMMATRIX camera;
	DirectX::XMMATRIX projection;
	PointLight* light;

private:
	// DX objects
	Microsoft::WRL::ComPtr<ID3D12Device5> pDevice;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> pCommandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> pSwapChain;
	// !manually writing the buffer count
	Microsoft::WRL::ComPtr<ID3D12Resource> pBackBuffers[2];
	Microsoft::WRL::ComPtr<ID3D12Resource> pDepthBuffer;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pCommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList6> pCommandList;
	// fence
	Microsoft::WRL::ComPtr<ID3D12Fence1> pFence;
	uint64_t fenceValue = 0;
	HANDLE fenceEvent;
	// viewport & scissor rect
	CD3DX12_RECT scissorRect;
	CD3DX12_VIEWPORT viewport;
	// rt and ds descriptor heaps
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> cbvsrvuavDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> imguiHeap;


	// rtv handle for the buffer used in frame
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsv;

	// current back buffer index
	UINT curBackBufferIndex = 0;
	// Width & Height
	uint16_t width;
	uint16_t height;
};