struct vs_out
{
    float4 position : SV_Position;
    float4 color : COLOR;
};
float4 pixel_shader(vs_out input) : SV_TARGET
{
    return input.color;
}