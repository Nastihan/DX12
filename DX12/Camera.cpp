#include "Camera.h"
#include "NastihanMath.h"
#include <algorithm>
#include "imgui/imgui.h"

Camera::Camera()
{
	Reset();

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
	namespace dx = DirectX;
	dx::XMStoreFloat3(&translation, dx::XMVector3Transform(
		dx::XMLoadFloat3(&translation),
		dx::XMMatrixRotationRollPitchYaw(pitch, yaw, 0.0f) 
	));
	pos = {
		pos.x + translation.x,
		pos.y + translation.y,
		pos.z + translation.z
	};

}

void Camera::Rotate(float dx, float dy)
{
	yaw = wrap_angle(yaw + dx * rotationSpeed);
	pitch = std::clamp(pitch + dy * rotationSpeed, 0.999f * -PI / 2.0f, 0.999f * PI / 2.0f);
}

void Camera::SpawnControlWindow() noexcept
{
	if (ImGui::Begin("Camera"))
	{
		ImGui::Text("Position");
		ImGui::SliderFloat("x", &pos.x, -100.0f, 100.0f, "%.1f");
		ImGui::SliderFloat("y", &pos.y, -100.0f, 100.0f);
		ImGui::SliderFloat("z", &pos.z, -100.0f, 100.0f);
		ImGui::Text("Orientation");
		ImGui::SliderAngle("Pitch", &pitch, -89.0f, 89.0f);
		ImGui::SliderAngle("Yaw", &yaw, -180.0f, 180.0f);
		if (ImGui::Button("Reset"))
		{
			Reset();
		}
	}
	ImGui::End();
}

void Camera::Reset() noexcept
{
	pos = { 0.0f, 7.0f, -25.0f };
	pitch = 0.0f;
	yaw = 0.0f;
}