
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
Texture2D<float4> target : register(u0);
ByteAddressBuffer start_offset_buffer : register(u1);
StructuredBuffer<Linker> linker_buffer : register(u2);


[numthreads(16, 16, 1)]
void CSMain(uint3 threadid : SV_DispatchThreadID)
{
    if(threadid.x<viewport_size.x && threadid.y<viewport_size.y)
    {
        int pixel_index = threadid.y * viewport_size.x + threadid.x;
        int linker_head = start_offset_buffer.Load(pixel_index * 4);
        while(linker_head!=-1)
        {
            Linker linker_node = linker_buffer[linker_head];
            
            
            linker_head = linker_node.next;

        }

    }
}