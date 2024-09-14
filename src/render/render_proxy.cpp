#include "render_proxy.h"
#include "d3d_helper.h"
#include "shading_model.h"
#include "device.h"
#include "descriptor_heap.h"

struct TriangleDeviceStaticResource
{
    Microsoft::WRL::ComPtr<ID3D12Resource> m_vertex_buffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertex_buffer_view;
};

void TriangleRenderProxy::InitRenderResources(std::shared_ptr<D3DHelper::Device> device)
{
	assert(device_static_resource ==nullptr);
    assert(b_render_resources_inited == false);
    m_device = device;
    device_static_resource = std::make_shared<TriangleDeviceStaticResource>();


    ThrowIfFailed(device->GetDevice()->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(vertex.size() * sizeof(Vertex)),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&device_static_resource->m_vertex_buffer)));
    
    device_static_resource->m_vertex_buffer_view.BufferLocation = device_static_resource->m_vertex_buffer->GetGPUVirtualAddress();
    device_static_resource->m_vertex_buffer_view.StrideInBytes = sizeof(Vertex);
    device_static_resource->m_vertex_buffer_view.SizeInBytes = vertex.size()* sizeof(Vertex);

    b_render_resources_inited = true;
    return;
}
void TriangleRenderProxy::UploadDynamic(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,int game_frame)
{
    return;
}
void TriangleRenderProxy::UploadStatic(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list)
{
    UINT8* pVertexDataBegin;
    CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
    ThrowIfFailed(device_static_resource->m_vertex_buffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
    memcpy(pVertexDataBegin, vertex.data(), vertex.size() * sizeof(Vertex));
    device_static_resource->m_vertex_buffer->Unmap(0, nullptr);
    return;
}

void TriangleRenderProxy::CommitParams(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, D3DHelper::StaticDescriptorStack* param_stack) const
{
    command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    command_list->IASetVertexBuffers(0, 1, &device_static_resource->m_vertex_buffer_view);
}

int TriangleRenderProxy::GetVertexCountPerInstance() const
{
    return vertex.size();
}