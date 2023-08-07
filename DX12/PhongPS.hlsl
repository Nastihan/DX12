#include "ShaderOps.hlsli"
#include "LightVectorData.hlsli"
#include "PointLight.hlsli"

struct PS_Input
{
    float3 viewFragPos : Position;
    float3 viewNormal : NORMAL;
    float4 pos : SV_Position;
};


static float specularColor = (1.0f,1.0f,1.0f);



float4 main(PS_Input input) : SV_TARGET
{
    float3 diffuse;
    float3 specular;
    
    // normalize the model normal
    input.viewNormal = normalize(input.viewNormal);
    
    // fragment to light vector data
    const LightVectorData lv = CalculateLightVectorData(viewLightPos, input.viewFragPos);
	    // attenuation
    const float att = Attenuate(attConst, attLin, attQuad, lv.distToL);
	    // diffuse
    diffuse = Diffuse(diffuseColor, diffuseIntensity, att, lv.dirToL, input.viewNormal);
        // specular
    specular = Speculate(
            diffuseColor * diffuseIntensity * specularColor, 0.2f, input.viewNormal,
            lv.vToL, input.viewFragPos, att, 3.0f
        );
    
    return float4(saturate((diffuse + ambient)  + specular), 1.0f);
}