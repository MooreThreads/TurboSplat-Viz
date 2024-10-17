RWByteAddressBuffer visible_clusters_num : register(u0);
RWBuffer<int> visible_clusters : register(u1);


[NumThreads(256, 1, 1)]
void main(
    uint tid : SV_DispatchThreadID,
    uint gid : SV_GroupID)
{
    visible_clusters[tid] = -1;
    if(tid==0)
    {
        visible_clusters_num.Store(0, 0);
    }

}