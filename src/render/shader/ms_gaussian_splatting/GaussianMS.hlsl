#include"struct_define.hlsli"

#define THREADSNUM 64

cbuffer view_cbuffer : register(b0)
{
    float4x4 view_transform;
    float4x4 project_transform;
    int2 viewport_size;
    float2 focal;
}

cbuffer batch_cbuffer : register(b1)
{
    float4x4 world_transform;
    int clusters_num;
}

StructuredBuffer<GaussianPoint> gaussian_points : register(t0);
StructuredBuffer<GaussianCluster> gaussian_clusters : register(t1);
Texture2D<float> gaussian_texture : register(t2);
ByteAddressBuffer visible_points_num : register(t3);
Buffer<uint> depth_order_pointid : register(t4);



[NumThreads(THREADSNUM, 1, 1)]
[OutputTopology("triangle")]
void main(
    uint tid : SV_DispatchThreadID,
    uint gtid : SV_GroupThreadID,
    uint gid : SV_GroupID,
    out indices uint3 tris[THREADSNUM*2],
    out vertices VertexOut verts[THREADSNUM*4]
)
{
    uint VertexCount = 0;
    uint TriangleCount = 0;
    if (gid*THREADSNUM < visible_points_num.Load(0))
    {
        uint valid_threads_num = min((visible_points_num.Load(0) - gid * THREADSNUM), THREADSNUM);
        VertexCount = valid_threads_num * 4;
        TriangleCount = valid_threads_num * 2;
    }
    SetMeshOutputCounts(VertexCount, TriangleCount);
    if (tid < visible_points_num.Load(0))
    {
        GaussianPoint gaussian_point = gaussian_points[depth_order_pointid[tid]];
        float4 world_pos = mul(world_transform, float4(gaussian_point.position, 1));
        float4 view_pos = mul(view_transform, world_pos);
        float4 homo_pos = mul(project_transform, view_pos);
        float4 ndc_pos = homo_pos / homo_pos.w;
        
    //proj 3d -> 2d
        float3x3 world_view_trans = mul(view_transform, world_transform);
        float3x3 ray_space_transform = float3x3(
        focal.x / view_pos.z, 0, -focal.x * view_pos.x / (view_pos.z * view_pos.z),
        0, focal.y / view_pos.z, -focal.y * view_pos.y / (view_pos.z * view_pos.z),
        0, 0, 0);
        float3x3 world_view_ray_trans = mul(ray_space_transform, world_view_trans);
        float2x2 cov2d = mul(mul(world_view_ray_trans, gaussian_point.cov3d), transpose(world_view_ray_trans));
        
    //eigen(vec2d)
        float det = cov2d[0][0] * cov2d[1][1] - cov2d[0][1] * cov2d[1][0];
        float mid = 0.5 * (cov2d[0][0] + cov2d[1][1]);
        float temp = sqrt(max((mid * mid - det), 1e-9f));
        float2 eigen_val = float2(mid - temp, mid + temp);
        float2 eigen_vec_0 = float2(1, 0);
        float2 eigen_vec_1 = float2(0, 1);
        if (cov2d[0][1] != 0)
        {
            float2 eigen_vec_y = (eigen_val - cov2d[0][0]) / cov2d[0][1];
            eigen_vec_0 = normalize(float2(1, eigen_vec_y.x));
            eigen_vec_1 = normalize(float2(1, eigen_vec_y.y));
        }
    //alpha
        float opacity_coefficient = 2 * log(255 * max(1 / 255, gaussian_point.color.w));
        
    //Ellipse
        float2 axis0 = eigen_vec_0 * sqrt(eigen_val.x * opacity_coefficient) / (viewport_size * 0.5);
        float2 axis1 = eigen_vec_1 * sqrt(eigen_val.y * opacity_coefficient) / (viewport_size * 0.5);
        if (ndc_pos.x < -1.3f || ndc_pos.x > 1.3f || ndc_pos.y < -1.3f || ndc_pos.y > 1.3f || ndc_pos.z < 0 || ndc_pos.z > 1 || gaussian_point.color.w < 1 / 255.0f)
        {
            axis0 = float2(1e-5, 1e-5);
            axis1 = axis0;
        }
        
    //2d gaussian mean
        float2 mean = float2(homo_pos.x / homo_pos.w, homo_pos.y / homo_pos.w);
        
        VertexOut vertex[4];
        vertex[0].PositionHS = homo_pos + float4(axis0 * homo_pos.w, 0, 0) + float4(axis1 * homo_pos.w, 0, 0);
        vertex[0].PositionHS.y = -vertex[0].PositionHS.y; //left hand
        vertex[0].UV = float2(0.0f, 0.0f);
        vertex[1].PositionHS = homo_pos - float4(axis0 * homo_pos.w, 0, 0) + float4(axis1 * homo_pos.w, 0, 0);
        vertex[1].UV = float2(0.0f, 1.0f);
        vertex[1].PositionHS.y = -vertex[1].PositionHS.y;
        vertex[2].PositionHS = homo_pos - float4(axis0 * homo_pos.w, 0, 0) - float4(axis1 * homo_pos.w, 0, 0);
        vertex[2].UV = float2(0.0f, 0.0f);
        vertex[2].PositionHS.y = -vertex[2].PositionHS.y;
        vertex[3].PositionHS = homo_pos + float4(axis0 * homo_pos.w, 0, 0) - float4(axis1 * homo_pos.w, 0, 0);
        vertex[3].UV = float2(1.0f, 0.0f);
        vertex[3].PositionHS.y = -vertex[3].PositionHS.y;
        vertex[0].color = gaussian_point.color;
        vertex[1].color = gaussian_point.color;
        vertex[2].color = gaussian_point.color;
        vertex[3].color = gaussian_point.color;
        
        
        tris[gtid * 2 + 0] = uint3(gtid * 4 + 1, gtid * 4 + 3, gtid * 4 + 0);
        tris[gtid * 2 + 1] = uint3(gtid * 4 + 3, gtid * 4 + 1, gtid * 4 + 2);
        verts[gtid * 4 + 0] = vertex[0];
        verts[gtid * 4 + 1] = vertex[1];
        verts[gtid * 4 + 2] = vertex[2];
        verts[gtid * 4 + 3] = vertex[3];

    }

}