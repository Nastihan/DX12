
struct VS_Input
{
    float3 pos : POSITION;
    float3 Color : COLOR;
};

struct VS_Output
{
    float4 Position : SV_Position;
    float4 Color : COLOR;
};

struct rotation
{
    matrix rot;
};
ConstantBuffer<rotation> rot : register(b0);

VS_Output main( VS_Input input )
{
    VS_Output output;
    output.Position = mul(float4(input.pos, 1.0f), rot.rot);
    output.Color = float4(input.Color, 1.0f);
    return output;
}