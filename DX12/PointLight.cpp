#include "PointLight.h"
#include <ranges>
#include "imgui/imgui.h"

PointLight::PointLight(Graphics& gfx)
	: mesh(gfx)
{
	Reset();

	// constant buffer
	const CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_DEFAULT };
	const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(cBufData));

	gfx.Device()->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&pLightCBuf)
	) >> chk;

	{
		const CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_UPLOAD };
		const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(cBufData));
		gfx.Device()->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&pUploadBuffer)
		) >> chk;
	}

	void* pLightData = nullptr;
	pUploadBuffer->Map(0, nullptr, &pLightData) >> chk;
	memcpy(pLightData, &cBufData,sizeof(PointLightCBuf));
	pUploadBuffer->Unmap(0,nullptr);

	gfx.ResetCmd();
	gfx.CommandList()->CopyResource(pLightCBuf.Get(), pUploadBuffer.Get());
	gfx.Execute();
	gfx.Sync();


	// descriptor heap for the shader resource view
	{
		const D3D12_DESCRIPTOR_HEAP_DESC heapDesc{
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			.NumDescriptors = 1,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		};
		gfx.Device()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pHeap)) >> chk;
	}
	// create handle to the srv heap and to the only view in the heap 
	D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = pHeap->GetCPUDescriptorHandleForHeapStart();
	// create the descriptor in the heap 
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = pLightCBuf->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = (UINT)sizeof(cBufData); 
		gfx.Device()->CreateConstantBufferView(&cbvDesc,cbvHandle);
	}
}

void PointLight::Draw(Graphics& gfx)
{
	mesh.SetPos(cBufData.pos);
	mesh.Draw(gfx);

}

void PointLight::Update(Graphics& gfx,DirectX::FXMMATRIX view)
{
	// Map the upload buffer and keep it mapped during the lifetime of the application
	void* pLightData = nullptr;
	pUploadBuffer->Map(0, nullptr, &pLightData) >> chk;
	auto dataCopy = cBufData;
	const auto pos = DirectX::XMLoadFloat3(&cBufData.pos);
	DirectX::XMStoreFloat3(&dataCopy.pos, DirectX::XMVector3Transform(pos, view));
	// Update the constant buffer data with the latest values
	memcpy(pLightData, &dataCopy, sizeof(PointLightCBuf));
	pUploadBuffer->Unmap(0, nullptr);

	// Copy the data from the upload buffer to the default heap (pLightCBuf)
	gfx.ResetCmd();
	gfx.CommandList()->CopyResource(pLightCBuf.Get(), pUploadBuffer.Get());
	gfx.Execute();
	gfx.Sync();
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

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> PointLight::GetHeap()
{
	return pHeap;
}
