#include"struct_define.hlsli"

#ifndef THREADSNUM
    #define THREADSNUM 256
#endif


cbuffer view_cbuffer : register(b0)
{
    float4 frustum_plane[6];
}

cbuffer batch_cbuffer : register(b1)
{
    int clusters_num;
}

StructuredBuffer<GaussianPoint> gaussian_points : register(t0);
StructuredBuffer<GaussianCluster> gaussian_clusters : register(t1);

RWByteAddressBuffer visible_clusters_num : register(u0);
RWBuffer<int> visible_clusters : register(u1);
RWByteAddressBuffer indirect_arg : register(u2);

bool inside_plane(float4 plane,float3 aabb_origin,float3 aabb_extend)
{
    float proj_origin = plane.x * aabb_origin.x + plane.y * aabb_origin.y + plane.z * aabb_origin.z + plane.w;
    float proj_ext = abs(plane.x * aabb_extend.x + plane.y * aabb_extend.y + plane.z * aabb_extend.z);
    float push_out_dist = proj_origin + proj_ext;
    return push_out_dist>0;
}

[NumThreads(THREADSNUM, 1, 1)]
void main(
    uint tid : SV_DispatchThreadID,
    uint gid : SV_GroupID)
{
    bool visible = false;
    if(tid<clusters_num)
    {
        //todo world transform
        visible = true;
        float3 aabb_origin = gaussian_clusters[tid].origin;
        float3 aabb_extend = gaussian_clusters[tid].extension;
        for (int i = 0; i < 6;i++)
        {
            float4 cur_plane = frustum_plane[i];
            visible&=inside_plane(cur_plane, aabb_origin, aabb_extend);
        }
    }
    
    //reduction
    uint laneAppendOffset = WavePrefixCountBits(visible);
    uint appendCount = WaveActiveCountBits(visible);
    uint appendOffset;
    if (WaveIsFirstLane())
    {
        // this way, we only issue one atomic for the entire wave, which reduces contention
        // and keeps the output data for each lane in this wave together in the output buffer
        visible_clusters_num.InterlockedAdd(0, appendCount, appendOffset);
        indirect_arg.InterlockedMax(0, ceil((appendOffset + appendCount) / (float) THREADSNUM));

    }
    appendOffset = WaveReadLaneFirst(appendOffset); // broadcast value
    appendOffset += laneAppendOffset; // and add in the offset for this lane
    
    if (visible)
    {
        visible_clusters[appendOffset] = tid;
    }
    
}