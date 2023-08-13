#include "TriangleRT.h"
#include <dxcapi.h>
#include "DXR/DXRHelper.h"
#include "DXR/BottomLevelASGenerator.h"
#include "DXR/RaytracingPipelineGenerator.h"
#include "DXR/RootSignatureGenerator.h"

TriangleRT::TriangleRT(Graphics& gfx)
{
	const std::vector<Vertex> vertices =
	{
			{ { 0.0f, 0.25f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
			{ { 0.25f, -0.25f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { -0.25f, -0.25f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
	};
	pVertexBuffer = std::make_unique<VertexBuffer>(gfx, vertices);
	// acceleration structures
	CreateAccelerationStructure(gfx);

	// create RayGen root signature
	nv_helpers_dx12::RootSignatureGenerator rscG;
	rscG.AddHeapRangesParameter(
		{ {0, 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_UAV,0},
		{0 , 1, 0, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1} }
	);
	pRayGenSignature = rscG.Generate(gfx.Device().Get(), true);
	// create RayHit root signature
	nv_helpers_dx12::RootSignatureGenerator rscH; 
	pRayHitSignature = rscH.Generate(gfx.Device().Get(), true);
	// create RayMiss root signature
	nv_helpers_dx12::RootSignatureGenerator rscM;
	pRayMissSignature = rscM.Generate(gfx.Device().Get(), true);

	// create RayTracing pipeline
	nv_helpers_dx12::RayTracingPipelineGenerator pipeline(gfx.Device().Get());
	pGenLibrary = nv_helpers_dx12::CompileShaderLibrary(L"ShadersRT\\RayGen.hlsl"); 
	pMissLibrary = nv_helpers_dx12::CompileShaderLibrary(L"ShadersRT\\Miss.hlsl");
	pHitLibrary = nv_helpers_dx12::CompileShaderLibrary(L"ShadersRT\\Hit.hlsl");

	pipeline.AddLibrary(pGenLibrary.Get(), { L"RayGen" });
	pipeline.AddLibrary(pMissLibrary.Get(), { L"Miss" });
	pipeline.AddLibrary(pHitLibrary.Get(), { L"ClosestHit" });

	pipeline.AddHitGroup(L"HitGroup", L"ClosestHit");
	pipeline.AddRootSignatureAssociation(pRayGenSignature.Get(), { L"RayGen" });
	pipeline.AddRootSignatureAssociation(pRayMissSignature.Get(), { L"Miss" });
	pipeline.AddRootSignatureAssociation(pRayHitSignature.Get(), { L"HitGroup" });

	pipeline.SetMaxPayloadSize(4 * sizeof(float));
	pipeline.SetMaxAttributeSize(2 * sizeof(float));
	pipeline.SetMaxRecursionDepth(1);

	pRTStateObject = pipeline.Generate();

	pRTStateObject->QueryInterface(IID_PPV_ARGS(&pRTStateObjectProperties));
}

// Create a bottom-level acceleration structure based on a list of vertex
// buffers in GPU memory along with their vertex count. The build is then done
// in 3 steps: gathering the geometry, computing the sizes of the required
// buffers, and building the actual AS
AccelerationStructureBuffers TriangleRT::CreateBottomLevelAS(Graphics& gfx, std::vector<std::pair<ComPtr<ID3D12Resource>, uint32_t>> vVertexBuffers)
{
	nv_helpers_dx12::BottomLevelASGenerator bottomLevelAS; 
	// Adding all vertex buffers and not transforming their position. 
	for (const auto &buffer : vVertexBuffers) 
	{ 
		bottomLevelAS.AddVertexBuffer(buffer.first.Get(), 0, buffer.second, sizeof(Vertex), 0, 0);
	} 
	// The AS build requires some scratch space to store temporary information. 
	// The amount of scratch memory is dependent on the scene complexity.
	UINT64 scratchSizeInBytes = 0; 
	// The final AS also needs to be stored in addition to the existing vertex buffers.
	// It size is also dependent on the scene complexity.
	 UINT64 resultSizeInBytes = 0;
	 bottomLevelAS.ComputeASBufferSizes(gfx.Device().Get(), false, &scratchSizeInBytes, &resultSizeInBytes);
	// Once the sizes are obtained, the application is responsible for allocating
	// the necessary buffers. Since the entire generation will be done on the GPU,
	// we can directly allocate those on the default heap 
	AccelerationStructureBuffers buffers;
	buffers.pScratch = nv_helpers_dx12::CreateBuffer( gfx.Device().Get(), scratchSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON, nv_helpers_dx12::kDefaultHeapProps);
	buffers.pResult = nv_helpers_dx12::CreateBuffer(gfx.Device().Get(), resultSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, nv_helpers_dx12::kDefaultHeapProps);
	// Build the acceleration structure. Note that this call integrates a barrier 
	// on the generated AS, so that it can be used to compute a top-level AS right after this method.
	bottomLevelAS.Generate(gfx.CommandList().Get(), buffers.pScratch.Get(), buffers.pResult.Get(), false, nullptr);
	return buffers;
}

// Create the main acceleration structure that holds all instances of the scene.
// Similarly to the bottom-level AS generation, it is done in 3 steps: gathering
// the instances, computing the memory requirements for the AS, and building the AS itself
void TriangleRT::CreateTopLevelAS(Graphics& gfx, const std::vector<std::pair<ComPtr<ID3D12Resource>, DirectX::XMMATRIX>>& instances)
{
	// Gather all the instances into the builder helper 
	for (size_t i = 0; i < instances.size(); i++) 
	{ 
		topLevelASGenerator.AddInstance(
			instances[i].first.Get(), instances[i].second, static_cast<UINT>(i), static_cast<UINT>(0)
		); 
	} 
	// As for the bottom-level AS, the building the AS requires some scratch space to store temporary data in addition to the actual AS.
	// In the case of the top-level AS, the instance descriptors also need to be stored in GPU memory.
	// This call outputs the memory requirements for each (scratch, results, instance descriptors) so that 
	// the application can allocate the corresponding memory 
	UINT64 scratchSize, resultSize, instanceDescsSize; 
	topLevelASGenerator.ComputeASBufferSizes(gfx.Device().Get(), true, &scratchSize, &resultSize, &instanceDescsSize);
	// Create the scratch and result buffers. Since the build is all done on GPU, those can be allocated on the default heap 
	topLevelASBuffers.pScratch = 
		nv_helpers_dx12::CreateBuffer( 
			gfx.Device().Get(), scratchSize,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON,
			nv_helpers_dx12::kDefaultHeapProps); 
	topLevelASBuffers.pResult
		= nv_helpers_dx12::CreateBuffer(
			gfx.Device().Get(), resultSize,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
			nv_helpers_dx12::kDefaultHeapProps); 
	// The buffer describing the instances: ID, shader binding information, matrices ...
	// Those will be copied into the buffer by the helper through mapping,
	// so the buffer has to be allocated on the upload heap.
	topLevelASBuffers.pInstanceDesc =
		nv_helpers_dx12::CreateBuffer(
			gfx.Device().Get(), instanceDescsSize,
			D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ,
			nv_helpers_dx12::kUploadHeapProps);
	// After all the buffers are allocated, or if only an update is required,
	// we can build the acceleration structure.
	// Note that in the case of the update we also pass the existing AS as the 'previous' AS,
	// so that it can be refitted in place.
	topLevelASGenerator.Generate(
		gfx.CommandList().Get(), topLevelASBuffers.pScratch.Get(),
		topLevelASBuffers.pResult.Get(), topLevelASBuffers.pInstanceDesc.Get());
}

// Combine the BLAS and TLAS builds to construct the entire acceleration
// structure required to raytrace the scene
void TriangleRT::CreateAccelerationStructure(Graphics& gfx)
{
	gfx.ResetCmd();
	// Build the bottom AS from the Triangle vertex buffer 
	AccelerationStructureBuffers bottomLevelBuffers = 
		CreateBottomLevelAS(gfx, {{pVertexBuffer->pVertexBuffer.Get(), 3}});
	// Just one instance for now 
	instances = {{bottomLevelBuffers.pResult, XMMatrixIdentity()}}; 
	CreateTopLevelAS(gfx, instances); 
	// Flush the command list and wait for it to finish 
	gfx.Execute();
	gfx.Sync();
	// Store the AS buffers. The rest of the buffers will be released once we exit the function
	bottomLevelAS = bottomLevelBuffers.pResult;
}
