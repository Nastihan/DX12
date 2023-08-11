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
		alignas(16) DirectX::XMFLOAT3 pos = { 0.0f,0.0f,0.0f };
		alignas(16) DirectX::XMFLOAT3 ambient = { 0.02f,0.02f,0.02f };
		alignas(16) DirectX::XMFLOAT3 diffuseColor = { 1.0f,1.0f,1.0f };
		float diffuseIntensity = 1.0f;
		float attConst = 1.0f;
		float attLin = 0.045f;
		float attQuad = 0.0075f;
	};
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> pLightCBuf;
	Microsoft::WRL::ComPtr<ID3D12Resource> pUploadBuffer;
	std::unique_ptr<ConstantBuffer<PointLightCBuf>> pCBuf;
	PointLightCBuf cBufData ;
	mutable Sphere mesh;
};
