#include "TriangleRT.h"
#include <dxcapi.h>
#include "DXR/DXRHelper.h"
#include "DXR/BottomLevelASGenerator.h"
#include "DXR/RaytracingPipelineGenerator.h"
#include "DXR/RootSignatureGenerator.h"
#include "BindableInclude.h"

TriangleRT::TriangleRT(Graphics& gfx)
{
	const std::vector<Vertex> vertices =
	{
		{{0.0f, 0.25f , 0.0f}, {1.0f, 1.0f, 0.0f, 1.0f}}, 
		{{0.25f, -0.25f , 0.0f}, {0.0f, 1.0f, 1.0f, 1.0f}}, 
		{{-0.25f, -0.25f , 0.0f}, {1.0f, 0.0f, 1.0f, 1.0f}}
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
	rscG.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS, 0, 0, 1);
	pRayGenSignature = rscG.Generate(gfx.Device().Get(), true);
	// create RayHit root signature
	nv_helpers_dx12::RootSignatureGenerator rscH; 
	rscH.AddRootParameter(D3D12_ROOT_PARAMETER_TYPE_SRV);
	pRayHitSignature = rscH.Generate(gfx.Device().Get(), true);
	// create RayMiss root signature
	nv_helpers_dx12::RootSignatureGenerator rscM;
	pRayMissSignature = rscM.Generate(gfx.Device().Get(), true);

	// create RayTracing pipeline
	nv_helpers_dx12::RayTracingPipelineGenerator pipeline(gfx.Device().Get());
	ComPtr<IDxcBlob> pGenLibrary = nv_helpers_dx12::CompileShaderLibrary(L"ShadersRT\\RayGen.hlsl"); 
	ComPtr<IDxcBlob> pMissLibrary = nv_helpers_dx12::CompileShaderLibrary(L"ShadersRT\\Miss.hlsl");
	ComPtr<IDxcBlob> pHitLibrary = nv_helpers_dx12::CompileShaderLibrary(L"ShadersRT\\Hit.hlsl");

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

	// output buffer
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.DepthOrArraySize = 1; 
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; 
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	resDesc.Width = gfx.GetWidth();
	resDesc.Height = gfx.GetHeight(); 
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.MipLevels = 1; resDesc.SampleDesc.Count = 1; 
	gfx.Device()->CreateCommittedResource(
		&nv_helpers_dx12::kDefaultHeapProps, 
		D3D12_HEAP_FLAG_NONE, &resDesc, 
		D3D12_RESOURCE_STATE_COPY_SOURCE, nullptr, IID_PPV_ARGS(&pOutputResource)) >> chk;


  // Create a SRV/UAV/CBV descriptor heap. We need 2 entries - 1 UAV for the
  // raytracing output and 1 SRV for the TLAS
	PSrvUavHeap = nv_helpers_dx12::CreateDescriptorHeap(
		gfx.Device().Get(), 2, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true);

	// Get a handle to the heap memory on the CPU side, to be able to write the
	// descriptors directly
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle =
		PSrvUavHeap->GetCPUDescriptorHandleForHeapStart();

	// Create the UAV. Based on the root signature we created it is the first
	// entry. The Create View methods write the view information directly into
	// srvHandle
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	gfx.Device()->CreateUnorderedAccessView(pOutputResource.Get(), nullptr, &uavDesc,
		srvHandle);

	// Add the Top Level AS SRV right after the raytracing output buffer
	srvHandle.ptr += gfx.Device()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.RaytracingAccelerationStructure.Location =
		topLevelASBuffers.pResult->GetGPUVirtualAddress();
	// Write the acceleration structure view in the heap
	gfx.Device()->CreateShaderResourceView(nullptr, &srvDesc, srvHandle);

	// create SBT
	sbtHelper.Reset();

	D3D12_GPU_DESCRIPTOR_HANDLE srvUavHeapHandle = PSrvUavHeap->GetGPUDescriptorHandleForHeapStart();

	// The helper treats both root parameter pointers and heap pointers as void*,
	// while DX12 uses the
	// D3D12_GPU_DESCRIPTOR_HANDLE to define heap pointers. The pointer in this
	// struct is a UINT64, which then has to be reinterpreted as a pointer.
	auto heapPointer = reinterpret_cast<UINT64*>(srvUavHeapHandle.ptr);

	// The ray generation only uses heap data
	auto transform = std::make_unique<TransformCbuf>(*this);
	auto mvp = transform->GetTransforms(gfx);
	sbtHelper.AddRayGenerationProgram(L"RayGen", { heapPointer,(void*)&mvp });

	// The miss and hit shaders do not access any external resources: instead they
	// communicate their results through the ray payload
	sbtHelper.AddMissProgram(L"Miss", {});

	// Adding the triangle hit shader
	sbtHelper.AddHitGroup(L"HitGroup",
		{ (void*)(pVertexBuffer->pVertexBuffer->GetGPUVirtualAddress()) });

	// Compute the size of the SBT given the number of shaders and their
	// parameters
	uint32_t sbtSize = sbtHelper.ComputeSBTSize();

	// Create the SBT on the upload heap. This is required as the helper will use
	// mapping to write the SBT contents. After the SBT compilation it could be
	// copied to the default heap for performance.
	pSBT = nv_helpers_dx12::CreateBuffer(
		gfx.Device().Get(), sbtSize, D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATE_GENERIC_READ, nv_helpers_dx12::kUploadHeapProps);
	if (!pSBT) {
		throw std::logic_error("Could not allocate the shader binding table");
	}
	// Compile the SBT from the shader and parameters info
	sbtHelper.Generate(pSBT.Get(), pRTStateObjectProperties.Get());

}

void TriangleRT::Draw(Graphics& gfx) const 
{

	ID3D12DescriptorHeap* heaps[] = { PSrvUavHeap.Get() };
	gfx.CommandList()->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);

	// On the last frame, the raytracing output was used as a copy source, to Now we need to transition it to a UAV so that the shaders can write in it.
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		pOutputResource.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	gfx.CommandList()->ResourceBarrier(1, &barrier);

	D3D12_DISPATCH_RAYS_DESC rayDesc = {};
	rayDesc.RayGenerationShaderRecord.StartAddress = pSBT->GetGPUVirtualAddress();
	rayDesc.RayGenerationShaderRecord.SizeInBytes = sbtHelper.GetRayGenSectionSize();

	rayDesc.MissShaderTable.StartAddress = (pSBT->GetGPUVirtualAddress() + sbtHelper.GetMissSectionSize()) + 32;
	rayDesc.MissShaderTable.SizeInBytes = sbtHelper.GetMissSectionSize();
	rayDesc.MissShaderTable.StrideInBytes = sbtHelper.GetMissEntrySize();

	rayDesc.HitGroupTable.StartAddress = (pSBT->GetGPUVirtualAddress()
		+ sbtHelper.GetMissSectionSize() + sbtHelper.GetHitGroupSectionSize()) ;
	rayDesc.HitGroupTable.SizeInBytes = sbtHelper.GetHitGroupSectionSize();
	rayDesc.HitGroupTable.StrideInBytes = sbtHelper.GetHitGroupEntrySize();

	rayDesc.Width = gfx.GetWidth();
	rayDesc.Height = gfx.GetHeight();
	rayDesc.Depth = 1;

	gfx.CommandList()->SetPipelineState1(pRTStateObject.Get());
	gfx.CommandList()->DispatchRays(&rayDesc);

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		pOutputResource.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COPY_SOURCE);

	gfx.CommandList()->ResourceBarrier(1, &barrier);

}

DirectX::XMMATRIX TriangleRT::GetTransform() const noexcept
{
	auto updateRotationMatrix = []() -> DirectX::XMMATRIX
		{
			// Assuming rotationSpeed is the speed at which you want the triangle to rotate (in degrees per second)
			static float rotationSpeed = 23.0f; // Adjust this value to control rotation speed
			static float rotationAngle = 0.0f;
			static std::chrono::steady_clock::time_point prevTime = std::chrono::steady_clock::now();

			// Get the current time
			auto currentTime = std::chrono::steady_clock::now();

			// Calculate the time elapsed since the last frame
			float deltaTime = std::chrono::duration<float>(currentTime - prevTime).count();
			prevTime = currentTime;

			// Update the rotation angle
			rotationAngle += rotationSpeed * deltaTime;

			// Calculate the new rotation matrix
			DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(3.0f, 0.0f, 2.0f);
			DirectX::XMMATRIX rotationMatrix =
				DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(rotationAngle))
				* DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(rotationAngle))
				* DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(rotationAngle));

			return translation;
		};
	const auto model = updateRotationMatrix();

	return model;
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
