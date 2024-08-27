struct GaussianPoint
{
    float3 position;
    float4 color;
    float3x3 cov3d;
};

struct GaussianCluster
{
    uint points_num;
    uint point_offset;
};

struct VertexOut
{
    float4 PositionHS : SV_Position;
    float4 color:COLOR0;
};

struct PPLL_Node
{
    int primitive_id;
    uint next;
};