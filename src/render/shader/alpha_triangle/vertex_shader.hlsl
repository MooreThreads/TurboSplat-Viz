#include"vs_header.hlsli"
cbuffer view_cbuffer : register(b0)
{
	float4x4 view_transform;
	float4x4 project_transform;
    int2 viewport_size;
}

cbuffer batch_cbuffer : register(b1)
{
    float4x4 world_transform;
}


vs_out VSMain(vs_in input)
{
    vs_out output;
	
    float4 world_position = mul(world_transform, float4(input.position, 1.0));
    float4 trans_position = mul(view_transform, world_position);
    float4 homo_position = mul(project_transform, trans_position);
    output.position = homo_position;

    output.color = input.color;

    return output;
}