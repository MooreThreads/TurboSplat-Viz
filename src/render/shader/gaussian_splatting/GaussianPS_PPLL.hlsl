#include"create_ppll_common.hlsli"

float4 PSMain(VertexOut input):SV_Target0
{
    float alpha = input.color.w * gaussian_texture.Sample(g_sampler, input.UV);
    return float4(input.color.xyz, alpha);
}