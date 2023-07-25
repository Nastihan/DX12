#include "Graphics.h"
#include "GraphicsError.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include "d3dx12.h"

Graphics::Graphics(uint16_t width, uint16_t height)
	:
	width(width),height(height)
{
	constexpr UINT bufferCount = 2;

	// dxgi factory
	Microsoft::WRL::ComPtr<IDXGIFactory4> pdxgifactory;
	CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&pdxgifactory)) >> chk;
	Microsoft::WRL::ComPtr<IDXGIAdapter> pAdapter;
	pdxgifactory->EnumAdapters(1U, pAdapter.GetAddressOf());

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
}
