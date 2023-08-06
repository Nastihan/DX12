#pragma once
#include "Graphics.h"
#include "Sphere.h"

class PointLight
{
public:
	PointLight(Graphics& gfx);
	void Draw(Graphics& gfx);

private:
	struct PointLightCBuf
	{
		alignas(16) DirectX::XMFLOAT3 pos = { 0.0f,0.0f,0.0f };
		alignas(16) DirectX::XMFLOAT3 ambient = { 0.01f, 0.01f, 0.01f };
		alignas(16) DirectX::XMFLOAT3 diffuseColor = { 1.0f, 1.0f, 1.0f };
		float diffuseIntensity = 1.0f;
		float attConst = 1.0f;
		float attLin = 0.045f;
		float attQuad = 0.0075f;
	};
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> pLightCBuf;
	PointLightCBuf cBufData = {};
	mutable Sphere mesh;
};
