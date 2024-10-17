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
    float3 origin;
    float3 extension;
};

struct VertexOut
{
    float4 PositionHS : SV_Position;
    float2 UV : TEXCOORD0;
    float4 color:COLOR0;
};

struct Payload
{
    uint ClusterIndices[32];
};