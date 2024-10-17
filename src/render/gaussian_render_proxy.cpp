#include"render_proxy.h"
#include"d3d_helper.h"
#include"shading_model.h"
#include"device.h"
#include "descriptor_heap.h"

struct GaussianDeviceStaticResource {
    Microsoft::WRL::ComPtr<ID3D12Resource> m_points_buffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_clusters_buffer;
    const int points_srv_index = 0;
    const int clusters_srv_index = 1;
    D3DHelper::StaticDescriptorHeap descriptor_heap;
};

void GaussianRenderProxy::InitRenderResources(std::shared_ptr<D3DHelper::Device> device)
{
    assert(device_static_resource == nullptr);
    assert(device_upload_resource == nullptr);
    assert(b_render_resources_inited == false);
    m_device = device;
    device_static_resource = std::make_shared<GaussianDeviceStaticResource>();
    device_upload_resource = std::make_shared<GaussianDeviceStaticResource>();

    ThrowIfFailed(device->GetDevice()->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(points_buffer.size() * sizeof(GaussianPoint)),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&device_upload_resource->m_points_buffer)));

    ThrowIfFailed(device->GetDevice()->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(clusters_buffer.size() * sizeof(GaussianCluster)),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&device_upload_resource->m_clusters_buffer)));


    ThrowIfFailed(device->GetDevice()->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(points_buffer.size() * sizeof(GaussianPoint)),
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&device_static_resource->m_points_buffer)));

    ThrowIfFailed(device->GetDevice()->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(clusters_buffer.size() * sizeof(GaussianCluster)),
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&device_static_resource->m_clusters_buffer)));

    D3D12_DESCRIPTOR_HEAP_DESC srv_heap_desc = {};
    srv_heap_desc.NumDescriptors = 2;
    srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    device_static_resource->descriptor_heap.Init(srv_heap_desc,m_device);


    D3D12_SHADER_RESOURCE_VIEW_DESC desc;
    desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.Buffer.FirstElement = 0;
    desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    desc.Buffer.NumElements = points_buffer.size();
    desc.Buffer.StructureByteStride = sizeof(GaussianPoint);
    device->GetDevice()->CreateShaderResourceView(device_static_resource->m_points_buffer.Get(), &desc, device_static_resource->descriptor_heap[device_static_resource->points_srv_index]);
    desc.Buffer.NumElements = clusters_buffer.size();
    desc.Buffer.StructureByteStride = sizeof(GaussianCluster);
    device->GetDevice()->CreateShaderResourceView(device_static_resource->m_clusters_buffer.Get(), &desc, device_static_resource->descriptor_heap[device_static_resource->clusters_srv_index]);

    b_render_resources_inited = true;
    return;
}

void GaussianRenderProxy::UploadStatic(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list)
{
    UINT8* pVertexDataBegin;
    CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
    ThrowIfFailed(device_upload_resource->m_points_buffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
    memcpy(pVertexDataBegin, points_buffer.data(), points_buffer.size() * sizeof(GaussianPoint));
    device_upload_resource->m_points_buffer->Unmap(0, nullptr);
    ThrowIfFailed(device_upload_resource->m_clusters_buffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
    memcpy(pVertexDataBegin, clusters_buffer.data(), clusters_buffer.size() * sizeof(GaussianCluster));
    device_upload_resource->m_clusters_buffer->Unmap(0, nullptr);
    
    command_list->CopyResource(device_static_resource->m_points_buffer.Get(), device_upload_resource->m_points_buffer.Get());
    command_list->CopyResource(device_static_resource->m_clusters_buffer.Get(), device_upload_resource->m_clusters_buffer.Get());

    D3D12_RESOURCE_BARRIER postCopyBarriers[2];
    postCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(device_static_resource->m_points_buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
    postCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(device_static_resource->m_clusters_buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
    command_list->ResourceBarrier(_countof(postCopyBarriers), postCopyBarriers);
    return;
}



int GaussianRenderProxy::GetVertexCountPerInstance() const
{
    return points_buffer.size();
}

int GaussianRenderProxy::GetClusterCountPerInstance() const
{
    return clusters_buffer.size();
}

void GaussianRenderProxy::CommitParams(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, D3DHelper::StaticDescriptorStack* param_stack) const
{
    param_stack[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].push_back(device_static_resource->descriptor_heap,0, device_static_resource->descriptor_heap.GetDescriptorNum());
}