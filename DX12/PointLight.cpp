#include "PointLight.h"
#include <ranges>
#include "imgui/imgui.h"

PointLight::PointLight(Graphics& gfx)
	: mesh(gfx)
{
	//Reset();

	pCBuf = std::make_unique<ConstantBuffer<PointLightCBuf>>(gfx, cBufData);
}

void PointLight::Draw(Graphics& gfx)
{
	
	mesh.SetPos(cBufData.pos);
	mesh.Draw(gfx);

}

void PointLight::Update(Graphics& gfx,DirectX::FXMMATRIX view)
{

	pCBuf->UpdateLight(gfx, cBufData,view);
}

void PointLight::Reset()
{
	cBufData.pos = { 0.0f,0.0f,0.0f };
	cBufData.ambient = { 0.02f,0.02f,0.02f };
	cBufData.diffuseColor = { 1.0f,1.0f,1.0f };
	cBufData.diffuseIntensity = 1.0f;
	cBufData.attConst = 1.0f;
	cBufData.attLin = 0.045f;
	cBufData.attQuad = 0.0075f;
}

void PointLight::SpawnControlWindow()
{
	if (ImGui::Begin("Light"))
	{

		ImGui::Text("Position");
		ImGui::SliderFloat("X", &cBufData.pos.x, -70.0f, 70.0f);
		ImGui::SliderFloat("Y", &cBufData.pos.y, -70.0f, 70.0f);
		ImGui::SliderFloat("Z", &cBufData.pos.z, -70.0f, 70.0f);
		ImGui::Text("Intensity/Color");
		ImGui::SliderFloat("Intensity", &cBufData.diffuseIntensity, 0.01f, 2.0f, "%.2f");
		ImGui::ColorEdit3("Diffuse Color", &cBufData.diffuseColor.x);
		ImGui::ColorEdit3("Ambient", &cBufData.ambient.x);
		ImGui::Text("Falloff");
		ImGui::SliderFloat("Constant", &cBufData.attConst, 0.05f, 10.0f, "%.2f");
		ImGui::SliderFloat("Linear", &cBufData.attLin, 0.0001f, 4.0f, "%.4f");
		ImGui::SliderFloat("Quadratic", &cBufData.attQuad, 0.0000001f, 10.0f, "%.7f");
		
		if (ImGui::Button("Reset"))
		{
			Reset();
		}

	}
	ImGui::End();
}

