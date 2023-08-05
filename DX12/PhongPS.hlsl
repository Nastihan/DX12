#include "ShaderOps.hlsli"
#include "LightVectorData.hlsli"

struct PS_Input
{
    float3 viewFragPos : Position;
    float3 viewNormal : NORMAL;
    float4 pos : SV_Position;
};

static float3 ambient = (0.04f, 0.04f, 0.04f);
static float3 diffuseColor = (1.0f, 1.0f, 1.0f);
static float diffuseIntensity = 2.0f;
static float specularColor = (1.0f,1.0f,1.0f);
static float specularIntensity = 0.8f;
static float attConst = 1.0f;
static float attLin = 0.05;
static float attQuad = 0.005;

static float3 lightpos = (6.0f, 1.0f, 3.0f);


float4 main(PS_Input input) : SV_TARGET
{
    float3 diffuse;
    float3 specular;
    
    // normalize the model normal
    input.viewNormal = normalize(input.viewNormal);
    
    // fragment to light vector data
    const LightVectorData lv = CalculateLightVectorData(lightpos, input.viewFragPos);
	    // attenuation
    const float att = Attenuate(attConst, attLin, attQuad, lv.distToL);
	    // diffuse
    diffuse = Diffuse(diffuseColor, diffuseIntensity, att, lv.dirToL, input.viewNormal);
        // specular
    specular = Speculate(
            diffuseColor * diffuseIntensity * specularColor, 0.8f, input.viewNormal,
            lv.vToL, input.viewFragPos, att, 4.0f
        );
    
    return float4(saturate((diffuse + ambient)  + specular), 1.0f);
}