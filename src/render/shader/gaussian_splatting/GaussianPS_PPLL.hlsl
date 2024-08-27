#include"create_ppll_common.hlsli"

float4 PSMain(VertexOut input):SV_Target0
{
    int2 pixel_coord = input.PositionHS.xy;
    uint new_head_linker_index = node_buffer.IncrementCounter();
    uint previous_head_linker_index;
    uint pixel_idx = (pixel_coord.y * viewport_size.x + pixel_coord.x);
    start_offset_buffer.InterlockedExchange(pixel_idx*4, new_head_linker_index, previous_head_linker_index);
    
    PPLL_Node ppll_node;
    ppll_node.next = previous_head_linker_index;
    ppll_node.primitive_id = pixel_idx; //todo
    node_buffer[new_head_linker_index] = ppll_node;
    
    return float4(0, 0, 0, 0);
}