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
static float specularColor = (1.0f, 1.0f, 1.0f);

static float3 albedoMesh = (0.5f, 0.5f, 0.5f);
static float3 emissivtyMesh = (0.1f, 0.1f, 0.1f);
static float roughness = 0.3f;
static float3 baseReflectane = (0.2f, 0.2f,0.2f);



float4 main(PS_Input input) : SV_TARGET
{

    //
    const float3 N = normalize(input.viewNormal);
    const float3 V = normalize(input.viewFragPos);
    const float3 L = normalize(viewLightPos - input.viewFragPos);
    const float3 H = normalize(V + L);

    float3 Ks = F(baseReflectane, V, H);
    float3 Kd = float3(1.0f,1.0f,1.0f) - Ks;
    
    float3 Lambert = albedoMesh / 3.14159265f;
    
    float3 cookTorranceNumerator = D(roughness, N, H) * G(roughness, N, V, L) * F(baseReflectane, V, H);
    float3 cookTorranceDenominator = 4.0f * max(dot(V, N), 0.0f) * max(dot(L, N), 0.0f);
    cookTorranceDenominator = max(cookTorranceDenominator, 0.000001);
    float3 cookTorrance = cookTorranceNumerator / cookTorranceDenominator;
    
    float3 BRDF = Kd * Lambert + cookTorrance;
    float3 outgoingLight = emissivtyMesh + BRDF * diffuseColor * max(dot(L, N), 0.0f);
    
    return float4(outgoingLight, 1.0f);
 
}