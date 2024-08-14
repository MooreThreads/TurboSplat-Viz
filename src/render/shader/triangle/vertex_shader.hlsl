cbuffer mvp_buffer : register(b1)
{
	float4x4 world_transform;
	float4x4 view_transform;
	float4x4 project_transform;
}

struct vs_in
{
    float3 position : POSITIONT;
    float4 color : COLOR;
};

struct vs_out
{
    float4 position : SV_Position;
    float4 color : COLOR;
};


vs_out vertex_shader(vs_in input)
{
    vs_out output;
	
    float4 world_position = mul(world_transform, float4(input.position, 1.0));
    float4 trans_position = mul(view_transform, world_position);
    float4 homo_position = mul(project_transform, trans_position);
    output.position = homo_position;

    output.color = input.color;

    return output;
}