RWByteAddressBuffer visible_clusters_counter : register(u0);
RWByteAddressBuffer visible_points_counter : register(u1);

RWStructuredBuffer<uint3> indirect_arg_filldata : register(u2);

[NumThreads(32, 1, 1)]
void main(uint tid : SV_DispatchThreadID)
{
    if(tid==0)
    {
        visible_clusters_counter.Store(0, 0);
        visible_points_counter.Store(0, 0);
        indirect_arg_filldata[0] = uint3(0, 1, 1);
    }

}