struct vs_out
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

struct ps_out
{
    float4 out_color : SV_TARGET;
    float out_depth : SV_Depth;
};
