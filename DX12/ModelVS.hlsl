
struct VS_Input
{
    float3 pos : POSITION;
    float3 n : NORMAL;
};

struct VS_Output
{
    float4 Position : SV_Position;
    float3 n : NORMAL;
};

struct MVP
{
    matrix transform;
};
ConstantBuffer<MVP> mvp : register(b0);

VS_Output main( VS_Input input )
{
    VS_Output output;
    output.Position = mul(float4(input.pos, 1.0f), mvp.transform);
    output.n = input.n;
    return output;
}