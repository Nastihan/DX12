
struct VS_Input
{
    float3 pos : POSITION;
    float2 tc : TEXCOORD;
};

struct VS_Output
{
    float4 Position : SV_Position;
    float2 tc : TEXCOORD;
};

struct MVP
{
    matrix model;
    matrix modelView;
    matrix modelViewProj;
};
ConstantBuffer<MVP> mvp : register(b0);

VS_Output main( VS_Input input )
{
    VS_Output output;
    output.Position = mul(float4(input.pos, 1.0f), mvp.modelViewProj);
    output.tc = input.tc;
    return output;
}