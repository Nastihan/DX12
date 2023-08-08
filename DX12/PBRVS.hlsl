
struct VS_Input
{
    float3 pos : POSITION;
    float3 n : NORMAL;
};

struct VS_Output
{
    float3 viewPos : Position;
    float3 viewNormal : NORMAL;
    float4 pos : SV_Position;
};

struct MVP
{
    matrix model;
    matrix modelView;
    matrix modelViewProj;
};
ConstantBuffer<MVP> mvp : register(b0);

VS_Output main(VS_Input input)
{
    VS_Output output;
    output.viewPos = (float3) mul(float4(input.pos, 1.0f), mvp.modelView);
    output.viewNormal = mul(input.n, (float3x3) mvp.modelView);
    output.pos = mul(float4(input.pos, 1.0f), mvp.modelViewProj);
    return output;
}