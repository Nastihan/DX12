struct MVP
{
    matrix model;
    matrix modelView;
    matrix modelViewProj;
};
ConstantBuffer<MVP> mvp : register(b0);

float4 main( float3 pos : POSITION ) : SV_POSITION
{
    return mul(float4(pos,1.0f), mvp.modelViewProj);
}