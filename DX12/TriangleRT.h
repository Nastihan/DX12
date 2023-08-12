#pragma once
#include <dxcapi.h>
#include "Graphics.h"
#include "DXR/TopLevelASGenerator.h"

using Microsoft::WRL::ComPtr;

class TriangleRT
{
public:
	AccelerationStructureBuffers CreateBottomLevelAS(std::vector<std::pair<ComPtr<ID3D12Resource>, uint32_t>> vVertexBuffers);
	void CreateTopLevelAS(const std::vector<std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>>& instances);
	void CreateAccelerationStructure();

private:
	ComPtr<ID3D12Resource> bottomLevelAS; // storage for the bottom level AS
	nv_helpers_dx12::TopLevelASGenerator topLevelASGenerator;
	AccelerationStructureBuffers topLevelASBuffers;
	std::vector<std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>> instances;
};

struct AccelerationStructureBuffers
{
	Microsoft::WRL::ComPtr<ID3D12Resource> pScratch; // Scratch memory for AS builder
	Microsoft::WRL::ComPtr<ID3D12Resource> pResult; // Where the AS is
	Microsoft::WRL::ComPtr<ID3D12Resource> pInstanceDesc; // Hold the matrices of the instances

};