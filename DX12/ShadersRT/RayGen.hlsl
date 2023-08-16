#include "Common.hlsl"

// Raytracing output texture, accessed as a UAV
RWTexture2D<float4> gOutput : register(u0);

// Raytracing acceleration structure, accessed as a SRV
RaytracingAccelerationStructure SceneBVH : register(t0);

// MVP transformation matrix
struct MVP
{
    float4x4 View;
    float4x4 Proj;
    float4x4 ViewI;
    float4x4 ProjI;
};
ConstantBuffer<MVP> mvp : register(b0);

[shader("raygeneration")] void RayGen() {
  // Initialize the ray payload
  HitInfo payload;
  payload.colorAndDistance = float4(0, 0, 0, 0);

  // Get the location within the dispatched 2D grid of work items
  // (often maps to pixels, so this could represent a pixel coordinate).
  uint2 launchIndex = DispatchRaysIndex().xy;
  float2 dims = float2(DispatchRaysDimensions().xy);
  float2 d = (((launchIndex.xy + 0.5f) / dims.xy) * 2.f - 1.f);
  // Define a ray, consisting of origin, direction, and the min-max distance
  // values
    RayDesc ray;
    ray.Origin = float3(d.x, -d.y, 1);
    ray.Direction = float3(0, 0, -1);
    ray.TMin = 0;
    ray.TMax = 100000;

  // Trace the ray
    TraceRay(SceneBVH, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, payload);
  gOutput[launchIndex] = float4(payload.colorAndDistance.rgb, 1.f);
}
