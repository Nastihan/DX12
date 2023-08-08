#include "ShaderOps.hlsli"
#include "LightVectorData.hlsli"
#include "PointLight.hlsli"
#include "PBR.hlsli"

struct PS_Input
{
    float3 viewFragPos : Position;
    float3 viewNormal : NORMAL;
    float4 pos : SV_Position;
};

static const float3 albedoMesh = float3(1.0f, 0.0f, 0.0f);
static const float3 emissivtyMesh = float3(0.0f, 0.0f, 0.0f);
static const float roughness = 0.3f;
static const float3 baseReflectane = float3(0.4f, 0.4f, 0.4f);



float4 main(PS_Input input) : SV_TARGET
{

    //
    LightVectorData lvData = CalculateLightVectorData(viewLightPos, input.viewFragPos);
    const float3 N = normalize(input.viewNormal);
    const float3 V = normalize(float3(0.0f, 0.0f, 0.0f) - input.viewFragPos);
    const float3 L = lvData.dirToL;
    const float3 H = normalize(V + L);

    float3 Ks = F(baseReflectane, V, H);
    float3 Kd = 1.0f - Ks;
    
    float3 Lambert = albedoMesh / 3.141592f;
    
    float3 cookTorranceNumerator = D(roughness, N, H) * G(roughness, N, V, L) * F(baseReflectane, V, H);
    float3 cookTorranceDenominator = 4.0f * max(dot(V, N), 0.0f) * max(dot(L, N), 0.0f);
    cookTorranceDenominator = max(cookTorranceDenominator, 0.000001);
    float3 cookTorrance = cookTorranceNumerator / cookTorranceDenominator;
    
    float3 BRDF = Kd * Lambert + cookTorrance;
    
    float3 att = Attenuate(attConst, attLin, attQuad, lvData.distToL);
    float3 lightIntensity =  att * diffuseColor * diffuseIntensity;
    float3 outgoingLight = emissivtyMesh + BRDF * lightIntensity  * max(dot(L, N), 0.0f);
    
    return float4(outgoingLight.x,outgoingLight.y,outgoingLight.z, 1.0f);
 
}