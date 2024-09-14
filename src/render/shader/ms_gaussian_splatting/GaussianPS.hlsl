#include"struct_define.hlsli"

Texture2D<float> gaussian_texture : register(t2);

SamplerState g_sampler : register(s0);

float4 PSMain(VertexOut input):SV_Target0
{
    float alpha = input.color.w * gaussian_texture.Sample(g_sampler, input.UV);
    return float4(input.color.xyz, alpha);
}