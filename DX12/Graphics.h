#pragma once
#include <cstdint>
#include <wrl.h>
#include <d3d12.h>

class Graphics
{
public:
	Graphics(uint16_t width, uint16_t height);
private:
	// DX objects
	Microsoft::WRL::ComPtr<ID3D12Device2> pDevice;

	uint16_t width;
	uint16_t height;
};