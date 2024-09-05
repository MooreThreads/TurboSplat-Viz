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

struct FragmentData
{
    uint color;
    float depth;
};

struct PPLL_Node
{
    FragmentData data;
    int next;
};

uint pack_color(float4 color_in)
{
    uint ret;
    ret = (uint(color_in.x * 256) << 24) + (uint(color_in.y * 256) << 16) + (uint(color_in.z * 256) << 8) + uint(color_in.w * 256);
    return ret;
}

float4 unpack_color(uint packed_color)
{
    float4 ret;
    ret.x = ((packed_color & 0xff000000) >> 24);
    ret.y = ((packed_color & 0x00ff0000) >> 16);
    ret.z = ((packed_color & 0x0000ff00) >> 8);
    ret.w = (packed_color & 0x000000ff);
    return ret / 256.0f;
}