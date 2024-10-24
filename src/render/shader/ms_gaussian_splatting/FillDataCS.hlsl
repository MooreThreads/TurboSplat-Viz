#include"struct_define.hlsli"

cbuffer view_cbuffer : register(b0)
{
    float4x4 view_transform;
    float4x4 project_transform;
}

cbuffer batch_cbuffer : register(b1)
{
    float4x4 world_transform;
}

StructuredBuffer<GaussianPoint> gaussian_points : register(t0);
StructuredBuffer<GaussianCluster> gaussian_clusters : register(t1);
ByteAddressBuffer visible_clusters_num : register(t2);
Buffer<int> visible_clusters : register(t3);

RWBuffer<int> depth_buffer : register(u0);
RWBuffer<int> point_id_buffer : register(u1);
RWByteAddressBuffer visible_points_num : register(u2);

[NumThreads(256, 1, 1)]
void main(uint tid : SV_DispatchThreadID)
{
    GaussianCluster cluster;
    cluster.points_num = 0;
    if (tid < visible_clusters_num.Load(0))
    {
        cluster=gaussian_clusters[visible_clusters[tid]];
    }
    uint offset_in_warp = WavePrefixSum(cluster.points_num);
    uint warp_points_num = WaveActiveSum(cluster.points_num);
    uint appendOffset = 0;
    if (WaveIsFirstLane())
    {
        visible_points_num.InterlockedAdd(0, warp_points_num, appendOffset);
    }
    appendOffset = WaveReadLaneFirst(appendOffset);
    appendOffset += offset_in_warp;
    
    for (int i = 0; i < cluster.points_num;i++)
    {
        GaussianPoint gaussian_point = gaussian_points[cluster.point_offset + i];
        float4 world_pos = mul(world_transform, float4(gaussian_point.position, 1));
        float4 view_pos = mul(view_transform, world_pos);
        float4 homo_pos = mul(project_transform, view_pos);
        uint depth = clamp(homo_pos.z / homo_pos.w, 0.0f, 1.0f) * uint(0xffffffff);
        
        point_id_buffer[appendOffset + i] = cluster.point_offset + i;
        depth_buffer[appendOffset + i] = depth;
    }

}

