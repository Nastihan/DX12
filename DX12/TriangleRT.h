#pragma once
#include "Graphics.h"
#include "DXR/TopLevelASGenerator.h"
#include <dxcapi.h>
#include "BindableInclude.h"

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

class TriangleRT
{
public:
	TriangleRT(Graphics& gfx);

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


};

