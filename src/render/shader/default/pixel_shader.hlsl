#include"ps_header.hlsli"
float4 PSMain(vs_out input) : SV_TARGET
{
    return input.color;
}