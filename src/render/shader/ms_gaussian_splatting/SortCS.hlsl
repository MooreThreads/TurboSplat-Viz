#include"struct_define.hlsli"

RWBuffer<int> depth : register(u0);
RWBuffer<int> point_id : register(u1);
RWByteAddressBuffer visible_points_num : register(u2);

StructuredBuffer<GaussianPoint> gaussian_points : register(t0);
StructuredBuffer<GaussianCluster> gaussian_clusters : register(t1);
ByteAddressBuffer visible_clusters_num : register(t2);
Buffer<int> visible_clusters : register(t3);

[NumThreads(256, 1, 1)]
void FillData(uint tid : SV_DispatchThreadID)
{
    GaussianCluster cluster;
    if (tid < visible_clusters_num.Load(0))
    {
        cluster=gaussian_clusters[visible_clusters[tid]];
    }
    
    
}


[NumThreads(256, 1, 1)]
void Sort(
    uint tid : SV_DispatchThreadID,
    uint gid : SV_GroupID)
{
    
    
    
}