#pragma once
#include "Graphics.h"
#include "DXR/TopLevelASGenerator.h"
#include <dxcapi.h>
#include "Drawable.h"
#include "BindableInclude.h"
#include "DXR/ShaderBindingTableGenerator.h"

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
	void CreateCameraBuffer(Graphics& gfx);

	void UpdateCameraBuffer(Graphics& gfx) const
	{
		std::vector<DirectX::XMMATRIX> matrices(4);

		// Initialize the view matrix, ideally this should be based on user
		// interactions The lookat and perspective matrices used for rasterization are
		// defined to transform world-space vertices into a [0,1]x[0,1]x[0,1] camera
		// space
		const DirectX::XMVECTOR Eye = DirectX::XMVectorSet(1.5f, 1.5f, 1.5f, 0.0f);
		const DirectX::XMVECTOR At = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		const DirectX::XMVECTOR Up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		matrices[0] = DirectX::XMMatrixLookAtRH(Eye, At, Up);

		const float fovAngleY = 45.0f * DirectX::XM_PI / 180.0f;
		matrices[1] =
			DirectX::XMMatrixPerspectiveFovRH(fovAngleY, 9.0f / 16.0f, 0.1f, 1000.0f);

		// Raytracing has to do the contrary of rasterization: rays are defined in
		// camera space, and are transformed into world space. To do this, we need to
		// store the inverse matrices as well.
		DirectX::XMVECTOR det;
		matrices[2] = DirectX::XMMatrixInverse(&det, matrices[0]);
		matrices[3] = DirectX::XMMatrixInverse(&det, matrices[1]);


		auto Transforms = std::make_unique<TransformCbuf>(*this);
		auto MATRICES = Transforms->GetTransformsRT(gfx);

		// Copy the matrix contents
		void* pData;
		m_cameraBuffer->Map(0, nullptr, &pData) >> chk;
		memcpy(pData, &MATRICES, m_cameraBufferSize);
		m_cameraBuffer->Unmap(0, nullptr);
	}

	ComPtr<ID3D12Resource> m_cameraBuffer;
	uint32_t m_cameraBufferSize = 0;
};

