#include"create_ppll_common.hlsli"


[NumThreads(128, 1, 1)]
[OutputTopology("triangle")]
void main(
    uint gtid : SV_GroupThreadID,
    uint gid : SV_GroupID,
    out indices uint3 tris[128],
    out vertices VertexOut verts[256]
)
{
    GaussianCluster cluster = gaussian_clusters[gid];

    uint VertexCount = cluster.points_num * 4;
    uint TriangleCount = cluster.points_num * 2;
    SetMeshOutputCounts(VertexCount, TriangleCount);

    if (gtid < cluster.points_num)
    {
        GaussianPoint gaussian_point = gaussian_points[cluster.point_offset + gtid];
        float4 world_pos = mul(world_transform, float4(gaussian_point.position, 1));
        float4 view_pos = mul(view_transform, world_pos);
        
        //proj 3d -> 2d
        float3x3 world_transform3x3 = world_transform;
        float3x3 world_cov3d = mul(mul(world_transform3x3, gaussian_point.cov3d), transpose(world_transform3x3)); //apply world transform
        float3x3 ray_space_transform = float3x3(
            focal.x / view_pos.z, 0, focal.x * view_pos.x / (view_pos.z * view_pos.z),
            0, focal.y / view_pos.z, focal.x * view_pos.x / (view_pos.z * view_pos.z),
            0, 0, 0
            );
        float2x2 cov2d = mul(mul(ray_space_transform, world_cov3d), transpose(ray_space_transform));
        
        //eigen(vec2d)
        float det = cov2d[0][0] * cov2d[1][1] - cov2d[0][1] * cov2d[1][0];
        float mid = 0.5 * (cov2d[0][0] + cov2d[1][1]);
        float temp = sqrt(max((mid * mid - det), 1e-9f));
        float2 eigen_val = float2(mid - temp, mid + temp);
        //TODO:float2x2 eigen_vec;
        float2 axis0 = float2(eigen_val.x / viewport_size.x, 0 / viewport_size.y);
        float2 axis1 = float2(0 / viewport_size.x, eigen_val.y / viewport_size.y);
        
        //2d gaussian mean
        float4 homo_pos = mul(project_transform, view_pos);
        float2 mean = float2(homo_pos.x / homo_pos.w, homo_pos.y / homo_pos.w);
        
        VertexOut vertex[4];
        /*vertex[0].PositionHS = homo_pos + float4(axis0 * homo_pos.w, 0, 0) + float4(axis1 * homo_pos.w, 0, 0);
        vertex[1].PositionHS = homo_pos - float4(axis0 * homo_pos.w, 0, 0) + float4(axis1 * homo_pos.w, 0, 0);
        vertex[2].PositionHS = homo_pos - float4(axis0 * homo_pos.w, 0, 0) - float4(axis1 * homo_pos.w, 0, 0);
        vertex[3].PositionHS = homo_pos + float4(axis0 * homo_pos.w, 0, 0) - float4(axis1 * homo_pos.w, 0, 0);*/
        int gtid_int = gtid;
        vertex[0].PositionHS = float4(gtid_int, gtid_int, 0.9f, 1);
        vertex[1].PositionHS = float4(gtid_int, gtid_int, 0.9f, 1);
        vertex[2].PositionHS = float4(gtid_int, gtid_int, 0.9f, 1);
        vertex[3].PositionHS = float4(gtid_int, gtid_int, 0.9f, 1);
        vertex[0].color = gaussian_point.color;
        vertex[1].color = gaussian_point.color;
        vertex[2].color = gaussian_point.color;
        vertex[3].color = gaussian_point.color;
        
        
        tris[gtid * 2 + 0] = uint3(gtid * 4 + 0, gtid * 4 + 1, gtid * 4 + 2);
        tris[gtid * 2 + 1] = uint3(gtid * 4 + 2, gtid * 4 + 3, gtid * 4 + 0);
        verts[gtid * 4 + 0] = vertex[0];
        verts[gtid * 4 + 1] = vertex[1];
        verts[gtid * 4 + 2] = vertex[2];
        verts[gtid * 4 + 3] = vertex[3];

    }

}