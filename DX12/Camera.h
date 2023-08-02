#pragma once
#include <DirectXMath.h>

class Camera
{
public:
	Camera();
	DirectX::XMMATRIX GetMatrix();
	void Translate(DirectX::XMFLOAT3 translation);
private:
	DirectX::XMMATRIX view;
	DirectX::XMFLOAT3 pos{0.0, 0.0f, -0.5f};
	float pitch;
	float yaw;
};