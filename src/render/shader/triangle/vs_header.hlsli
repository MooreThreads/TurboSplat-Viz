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