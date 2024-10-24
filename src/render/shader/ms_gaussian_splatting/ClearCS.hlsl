RWByteAddressBuffer visible_clusters_counter : register(u0);
RWByteAddressBuffer visible_points_counter : register(u1);

[NumThreads(32, 1, 1)]
void main(uint tid : SV_DispatchThreadID)
{
    if(tid==0)
    {
        visible_clusters_counter.Store(0, 0);
        visible_points_counter.Store(0, 0);
    }

}