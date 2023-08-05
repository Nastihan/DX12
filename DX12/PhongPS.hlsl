
struct PS_Input
{
    float3 viewFragPos : Position;
    float3 viewNormal : NORMAL;
    float4 pos : SV_Position;
};

float4 main(PS_Input input) : SV_TARGET
{
    return float4(0.2f, 0.0f, 0.2f, 1.0f);
}