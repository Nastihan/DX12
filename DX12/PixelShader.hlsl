Texture2D tex : register(t0);
SamplerState smplr : register(s0);

struct PS_Input
{
    float4 Position : SV_Position;
    float2 tc : TEXCOORD;
};

float4 main(PS_Input input) : SV_TARGET
{
    return tex.Sample(smplr, input.tc);
}