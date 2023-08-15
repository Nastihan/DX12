#include "Common.hlsl"

[shader("closesthit")] 
void ClosestHit(inout HitInfo payload, Attributes attrib) 
{
    uint2 launchIndex = DispatchRaysIndex().xy;
    float2 dims = float2(DispatchRaysDimensions().xy);
    float ramp = launchIndex.y / dims.y;
    payload.colorAndDistance = float4(0.0f, 0.9f, 0.0f , -1.0f);
}
