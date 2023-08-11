#pragma once
#include "Sphere.h"
#include "Graphics.h" 


class PointLight
{
public:
	PointLight() = default;
	PointLight(Graphics& gfx);
	void Draw(Graphics& gfx);
	void Update(Graphics& gfx, DirectX::FXMMATRIX view);
	void Reset();
	void SpawnControlWindow();

private:
	struct alignas(256) PointLightCBuf
	{
		alignas(16) DirectX::XMFLOAT3 pos;
		alignas(16) DirectX::XMFLOAT3 ambient;
		alignas(16) DirectX::XMFLOAT3 diffuseColor;
		float diffuseIntensity;
		float attConst;
		float attLin;
		float attQuad;
	};
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> pLightCBuf;
	Microsoft::WRL::ComPtr<ID3D12Resource> pUploadBuffer;
	PointLightCBuf cBufData = {};
	mutable Sphere mesh;
};
