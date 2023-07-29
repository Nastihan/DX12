
struct PS_Input
{
    float4 Position : SV_Position;
    float4 Color : COLOR;
};

float4 main(PS_Input input) : SV_TARGET
{
    return input.Color;
}