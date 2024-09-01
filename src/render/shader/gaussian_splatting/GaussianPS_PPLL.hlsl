#include"create_ppll_common.hlsli"

float4 PSMain(VertexOut input):SV_Target0
{
    float alpha = input.color.w * gaussian_texture.Sample(g_sampler, input.UV);
    if(alpha > 0.01f)
    {
        int2 pixel_coord = input.PositionHS.xy;
        uint new_head_linker_index = node_buffer.IncrementCounter();
        int previous_head_linker_index;
        uint pixel_idx = (pixel_coord.y * viewport_size.x + pixel_coord.x);
        start_offset_buffer.InterlockedExchange(pixel_idx * 4, new_head_linker_index, previous_head_linker_index);
    
        PPLL_Node ppll_node;
        ppll_node.next = previous_head_linker_index;
        ppll_node.data.depth = input.PositionHS.z;
        ppll_node.data.color = pack_color(float4(input.color.xyz, alpha));
        node_buffer[new_head_linker_index] = ppll_node;
    }
    
    return float4(0, 0, 0, 0);
}