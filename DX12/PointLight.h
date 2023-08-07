#pragma once
#include "Sphere.h"
#include "Graphics.h" 


class PointLight
{
public:
	PointLight() = default;
	PointLight(Graphics& gfx);
	void Draw(Graphics& gfx);
	void Reset();
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetHeap();

private:
	struct alignas(256) PointLightCBuf
	{
		DirectX::XMFLOAT3 pos = { 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT3 ambient = { 0.01f, 0.01f, 0.01f };
		DirectX::XMFLOAT3 diffuseColor = { 1.0f, 1.0f, 1.0f };
		float diffuseIntensity = 1.0f;
		float attConst = 1.0f;
		float attLin = 0.045f;
		float attQuad = 0.0075f;
	};
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> pLightCBuf;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pHeap;
	PointLightCBuf cBufData = {};
	mutable Sphere mesh;
};
