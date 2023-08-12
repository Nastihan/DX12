#include "TriangleRT.h"
#include "DXR/DXRHelper.h"
#include "DXR/BottomLevelASGenerator.h"
#include <dxcapi.h>



AccelerationStructureBuffers TriangleRT::CreateBottomLevelAS(std::vector<std::pair<ComPtr<ID3D12Resource>, uint32_t>> vVertexBuffers)
{
	return AccelerationStructureBuffers();
}

void TriangleRT::CreateTopLevelAS(const std::vector<std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>>& instances)
{
}

void TriangleRT::CreateAccelerationStructure()
{
}
