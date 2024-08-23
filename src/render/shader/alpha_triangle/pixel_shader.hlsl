#include"ps_header.hlsli"
cbuffer view_cbuffer : register(b0)
{
    float4x4 view_transform;
    float4x4 project_transform;
    int2 viewport_size;
}
struct Linker
{
    int primitive_id;
    uint next;
};
RWByteAddressBuffer start_offset_buffer : register(u1);
RWStructuredBuffer<Linker> linker_buffer : register(u2);
ps_out PSMain(vs_out input) 
{
    int2 pixel_coord = input.position.xy;
    uint new_head_linker_index = linker_buffer.IncrementCounter();
    uint previous_head_linker_index;
    uint pixel_idx = (pixel_coord.y * viewport_size.x + pixel_coord.x);
    start_offset_buffer.InterlockedExchange(pixel_idx*4, new_head_linker_index, previous_head_linker_index);
    
    Linker linknode;
    linknode.next = previous_head_linker_index;
    linknode.primitive_id = pixel_idx; //todo
    linker_buffer[new_head_linker_index] = linknode;
    
    ps_out result;
    result.out_color = input.color ;
    result.out_depth = input.position.z;
    return result;
}