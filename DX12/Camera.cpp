#include "Camera.h"
#include "NastihanMath.h"
#include <algorithm>

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
	const DirectX::XMVECTOR forwardBaseVector = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	const auto lookVector = DirectX::XMVector3Transform(forwardBaseVector,
		DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, 0.0f)
	);
	const auto camPosition = XMLoadFloat3(&pos);
	const auto camTarget = DirectX::XMVectorAdd(camPosition,lookVector);
	return DirectX::XMMatrixLookAtLH(camPosition, camTarget, DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

}

void Camera::Translate(DirectX::XMFLOAT3 translation)
{
	pos = {
		pos.x + translation.x,
		pos.y + translation.y,
		pos.z + translation.z
	};

}

void Camera::Rotate(float dx, float dy)
{
	yaw = wrap_angle(yaw + dx);
	pitch = std::clamp(pitch + dx, 0.995f * -PI / 2.0f, 0.995f * PI / 2.0f);
}