#include"struct_define.hlsli"
cbuffer view_cbuffer : register(b0)
{
    float4x4 view_transform;
    float4x4 project_transform;
    int2 viewport_size;
    float2 focal;
}

cbuffer batch_cbuffer : register(b1)
{
    float4x4 world_transform;
}

StructuredBuffer<GaussianPoint> gaussian_points : register(t0);
StructuredBuffer<GaussianCluster> gaussian_clusters : register(t1);
Texture2D<float> gaussian_texture : register(t2);

SamplerState g_sampler : register(s0);

RWByteAddressBuffer start_offset_buffer : register(u1);
RWStructuredBuffer<PPLL_Node> node_buffer : register(u2);//with counter
RWBuffer<int> counter : register(u3);