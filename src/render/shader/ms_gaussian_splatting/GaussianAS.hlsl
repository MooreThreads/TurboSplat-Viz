#include"struct_define.hlsli"
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
StructuredBuffer<GaussianCluster> gaussian_clusters : register(t1);


groupshared Payload s_Payload;

[NumThreads(32, 1, 1)]
void main(uint gtid : SV_GroupThreadID, uint tid : SV_DispatchThreadID, uint gid : SV_GroupID)
{
    bool visible = false;
    if(tid<clusters_num)
    {
        GaussianCluster cluster = gaussian_clusters[tid];
        float4 world_pos = mul(world_transform, float4(cluster.origin, 1));
        float4 view_pos = mul(view_transform, world_pos);
        float4 homo_pos = mul(project_transform, view_pos);
        float4 ndc_pos = homo_pos / homo_pos.w;
        visible = !(ndc_pos.x < -1.3f || ndc_pos.x > 1.3f || ndc_pos.y < -1.3f || ndc_pos.y > 1.3f || ndc_pos.z < 0 || ndc_pos.z > 1);
    }
    
    if (visible)
    {
        uint index = WavePrefixCountBits(visible);
        s_Payload.ClusterIndices[index] = tid;
    }
    uint visibleCount = WaveActiveCountBits(visible);
    DispatchMesh(visibleCount, 1, 1, s_Payload);
    
}