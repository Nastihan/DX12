
struct PS_Input
{
    float4 Position : SV_Position;
    float2 n : NORMAL;
};

float4 main(PS_Input input) : SV_TARGET
{
	return float4(0.2f, 0.0f, 0.2f, 1.0f);
}