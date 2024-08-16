#include"vs_header.hlsli"


vs_out VSMain(vs_in input)
{
    vs_out output;
    output.position = float4(input.position, 1);
    output.color = input.color;

    return output;
}