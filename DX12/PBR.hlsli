
// GGX/Trowbrudge_Reitz Normal Distribution Function
float D(float alpha, float3 N, float3 H)
{
    float numerator = pow(alpha, 2.0f);
    
    float NdotH = max(dot (N, H), 0.0f);
    float denominator = 3.141592f * pow(pow(NdotH, 2.0f) * (pow(alpha, 2.0f) - 1.0f) + 1.0f, 2.0f);
    denominator = max(denominator, 0.000001f);
    
    return numerator / denominator;
}
// Schlick-Beckmann Geometry Shadowing Function
float G1(float alpha, float3 N, float3 X)
{
    float numerator = max(dot(N, X), 0.0f);
    
    float k = alpha / 2.0f ;
    float denominator = max(dot(N, X), 0.0f) * (1.0f - k) + k;
    
    return numerator / denominator;
}
// Smith Model
float G(float alpha, float3 N, float3 V, float3 L)
{
    return G1(alpha, N, V) * G1(alpha, N, L);
}
// Fresnel-Schlick Function
float3 F(float3 F0, float3 V, float3 H)
{
    return F0 + (float3(1.0f,1.0f,1.0f) - F0) * pow(1 - max(dot(V, H), 0.0f), 5.0f);
}
