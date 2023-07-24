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
	Microsoft::WRL::ComPtr<IDXGIFactory4> dxgifactory;
	CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&dxgifactory)) >> chk;

	// device
	D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&pDevice));
}
