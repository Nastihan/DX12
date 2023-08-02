#include "Camera.h"

Camera::Camera()
{

	const auto eyePosition = DirectX::XMVectorSet(0, 0, -5, 1);
	DirectX::XMStoreFloat3(&pos, eyePosition);
	const auto focusPoint = DirectX::XMVectorSet(0, 0, 0, 1);
	const auto upDirection = DirectX::XMVectorSet(0, 1, 0, 0);
	view = DirectX::XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);
	
}

DirectX::XMMATRIX Camera::GetMatrix()
{
	const auto eyePosition = DirectX::XMVectorSet(pos.x, pos.y, pos.z, 1);
	DirectX::XMStoreFloat3(&pos, eyePosition);
	const auto focusPoint = DirectX::XMVectorSet(0, 0, 0, 1);
	const auto upDirection = DirectX::XMVectorSet(0, 1, 0, 0);
	view = DirectX::XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);

	return view;
}

void Camera::Translate(DirectX::XMFLOAT3 translation)
{
	pos = {
		pos.x + translation.x,
		pos.y + translation.y,
		pos.z + translation.z
	};

}
