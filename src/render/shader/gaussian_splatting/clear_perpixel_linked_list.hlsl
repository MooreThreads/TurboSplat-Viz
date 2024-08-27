#include"create_ppll_common.hlsli"
[numthreads(512, 1, 1)]
void CSMain(uint3 threadid : SV_DispatchThreadID)
{
    if(threadid.x==0)
    {
        counter[0] = 0;
    }
    if(threadid.x<1920*1080)//todo use macro
    {
        start_offset_buffer.Store(threadid.x*4, -1);
    }
}