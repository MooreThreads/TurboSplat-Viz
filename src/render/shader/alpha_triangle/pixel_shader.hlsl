#include"ps_header.hlsli"
ps_out PSMain(vs_out input) 
{
    ps_out result;
    result.out_color = input.color ;
    result.out_depth = input.position.z;
    return result;
}