#include"struct_define.hlsli"
cbuffer view_cbuffer : register(b0)
{
    float4x4 view_transform;
    float4x4 project_transform;
    int2 viewport_size;
    float2 focal;
}
Buffer<int> start_offset_buffer : register(t0);
StructuredBuffer<PPLL_Node> node_buffer : register(t1);
Texture2D<float> gaussian_texture : register(t2);

SamplerState g_sampler : register(s0);

RWTexture2D<float4> target:register(u0);


[NumThreads(16, 16, 1)]
void CSMain(uint3 tid : SV_DispatchThreadID)
{
    if(tid.x<viewport_size.x&&tid.y<viewport_size.y)
    {
        uint pixel_index = tid.y * viewport_size.x + tid.x;
        int node_index = start_offset_buffer[pixel_index];
        FragmentData sorted_fragments[8] = { { 0, 0.0f }, { 0, 0.0f }, { 0, 0.0f }, { 0, 0.0f }, { 0, 0.0f }, { 0, 0.0f }, { 0, 0.0f }, { 0, 0.0f } };
        while(node_index!=-1)
        {
            PPLL_Node node = node_buffer[node_index];
            float cur_alpha = (node.data.color & 0x000000ff) / 256.0f;
            if (cur_alpha>0.1f&&node.data.depth > sorted_fragments[7].depth)
            {
                int insert_slot = 7;
                for (; insert_slot >= 0 && node.data.depth > sorted_fragments[insert_slot - 1].depth; insert_slot--)
                {
                    sorted_fragments[insert_slot] = sorted_fragments[insert_slot - 1];
                }
                sorted_fragments[insert_slot] = node.data;
            }
            node_index = node.next;
        }
        
        float4 color = float4(0, 0, 0, 1.0);
        for (int i = 0; i < 8;i++)
        {
            float4 cur_color = unpack_color(sorted_fragments[i].color);
            color.xyz += cur_color.xyz * cur_color.w * color.w;
            color.w *= (1 - cur_color.w);
        }
        
        target[tid.xy] = color;
    }
    return;
}