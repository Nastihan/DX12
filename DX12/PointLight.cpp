#include "PointLight.h"

PointLight::PointLight(Graphics& gfx)
	: mesh(gfx)
{
	// descriptor heap for the shader resource view
	{
		const D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			.NumDescriptors = 1,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		};
		gfx.Device()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap)) >> chk;
	}
	// create handle to the srv heap and to the only view in the heap 
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = srvHeap->GetCPUDescriptorHandleForHeapStart();
	// create the descriptor in the heap 
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = sizeof(cBufData) / sizeof(UINT);
		srvDesc.Buffer.StructureByteStride = sizeof(cBufData);
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		gfx.Device()->CreateShaderResourceView(pLightCBuf.Get(), &srvDesc, srvHandle);
	}


}

void PointLight::Draw(Graphics& gfx)
{
	mesh.Draw(gfx);
}
