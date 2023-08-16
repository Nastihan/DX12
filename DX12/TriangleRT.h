#pragma once
#include "Graphics.h"
#include "DXR/TopLevelASGenerator.h"
#include <dxcapi.h>
#include "Drawable.h"
#include "BindableInclude.h"
#include <ShaderBindingTableGenerator.h>


using Microsoft::WRL::ComPtr;

struct AccelerationStructureBuffers
{
	Microsoft::WRL::ComPtr<ID3D12Resource> pScratch; // Scratch memory for AS builder
	Microsoft::WRL::ComPtr<ID3D12Resource> pResult; // Where the AS is
	Microsoft::WRL::ComPtr<ID3D12Resource> pInstanceDesc; // Hold the matrices of the instances
};

struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
};

class TriangleRT : public Drawable
{
public:
	TriangleRT(Graphics& gfx);
	void Draw(Graphics& gfx) const override;
	DirectX::XMMATRIX GetTransform() const noexcept override;

	ComPtr<ID3D12Resource> GetOutputBuffer()
	{
		return pOutputResource;
	}

	// passing a vector of pairs : first element is the resource holding the vertex buffer and the second element is the number of vertices
	AccelerationStructureBuffers CreateBottomLevelAS(Graphics& gfx, std::vector<std::pair<ComPtr<ID3D12Resource>, uint32_t>> vVertexBuffers);
	void CreateTopLevelAS(Graphics& gfx, const std::vector<std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>>& instances);
	void CreateAccelerationStructure(Graphics& gfx);

private:
	ComPtr<ID3D12Resource> bottomLevelAS; // storage for the bottom level AS
	nv_helpers_dx12::TopLevelASGenerator topLevelASGenerator;
	AccelerationStructureBuffers topLevelASBuffers;
	std::vector<std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>> instances;
	// vertex buffer
	std::unique_ptr<VertexBuffer> pVertexBuffer;
	// root signatures
	ComPtr<ID3D12RootSignature> pRayGenSignature;
	ComPtr<ID3D12RootSignature> pRayHitSignature;
	ComPtr<ID3D12RootSignature> pRayMissSignature;
	// pipeline state
	ComPtr<ID3D12StateObject> pRTStateObject;
	ComPtr<ID3D12StateObjectProperties> pRTStateObjectProperties;
	// output buffer 
	ComPtr<ID3D12Resource> pOutputResource;
	ComPtr<ID3D12DescriptorHeap> PSrvUavHeap;
	// SBT
	ComPtr<ID3D12Resource> pSBT;
	nv_helpers_dx12::ShaderBindingTableGenerator sbtHelper;

public:
	//void CreateCameraBuffer(Graphics& gfx) {
	//	uint32_t nbMatrix = 4; // view, perspective, viewInv, perspectiveInv
	//	m_cameraBufferSize = nbMatrix * sizeof(XMMATRIX);

	//	// Create the constant buffer for all matrices
	//	m_cameraBuffer = nv_helpers_dx12::CreateBuffer(
	//		gfx.Device().Get(), m_cameraBufferSize, D3D12_RESOURCE_FLAG_NONE,
	//		D3D12_RESOURCE_STATE_GENERIC_READ, nv_helpers_dx12::kUploadHeapProps);

	//	// #DXR Extra - Refitting
	//	// Create a descriptor heap that will be used by the rasterization shaders:
	//	// Camera matrices and per-instance matrices
	//	m_constHeap = nv_helpers_dx12::CreateDescriptorHeap(
	//		gfx.Device().Get(), 2, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true);

	//	// Describe and create the constant buffer view.
	//	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	//	cbvDesc.BufferLocation = m_cameraBuffer->GetGPUVirtualAddress();
	//	cbvDesc.SizeInBytes = m_cameraBufferSize;

	//	// Get a handle to the heap memory on the CPU side, to be able to write the
	//	// descriptors directly
	//	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle =
	//		m_constHeap->GetCPUDescriptorHandleForHeapStart();
	//	gfx.Device()->CreateConstantBufferView(&cbvDesc, srvHandle);

	//	// #DXR Extra - Refitting
	//	// Add the per-instance buffer
	//	srvHandle.ptr += gfx.Device()->GetDescriptorHandleIncrementSize(
	//		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//}
	// #DXR Extra: Perspective Camera
	//--------------------------------------------------------------------------------
	// Create and copies the viewmodel and perspective matrices of the camera
	//
	//void UpdateCameraBuffer() {
	//	std::vector<XMMATRIX> matrices(4);

	//	// Initialize the view matrix, ideally this should be based on user
	//	// interactions The lookat and perspective matrices used for rasterization are
	//	// defined to transform world-space vertices into a [0,1]x[0,1]x[0,1] camera
	//	// space
	//	const glm::mat4& mat = nv_helpers_dx12::CameraManip.getMatrix();
	//	memcpy(&matrices[0].r->m128_f32[0], glm::value_ptr(mat), 16 * sizeof(float));

	//	float fovAngleY = 45.0f * XM_PI / 180.0f;
	//	matrices[1] =
	//		XMMatrixPerspectiveFovRH(fovAngleY, m_aspectRatio, 0.1f, 1000.0f);

	//	// Raytracing has to do the contrary of rasterization: rays are defined in
	//	// camera space, and are transformed into world space. To do this, we need to
	//	// store the inverse matrices as well.
	//	XMVECTOR det;
	//	matrices[2] = XMMatrixInverse(&det, matrices[0]);
	//	matrices[3] = XMMatrixInverse(&det, matrices[1]);

	//	// Copy the matrix contents
	//	uint8_t* pData;
	//	ThrowIfFailed(m_cameraBuffer->Map(0, nullptr, (void**)&pData));
	//	memcpy(pData, matrices.data(), m_cameraBufferSize);
	//	m_cameraBuffer->Unmap(0, nullptr);
	//}

	ComPtr<ID3D12Resource> m_cameraBuffer;
	ComPtr<ID3D12DescriptorHeap> m_constHeap;
	uint32_t m_cameraBufferSize = 0;
};

