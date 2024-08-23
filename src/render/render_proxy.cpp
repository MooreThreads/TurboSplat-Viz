#include "render_proxy.h"
#include "d3d_helper.h"
#include "shading_model.h"

void TriangleRenderProxy::InitRenderResources()
{
	assert(device_static_resource ==nullptr);
    assert(b_render_resources_inited == false);
    device_static_resource = std::make_unique<DeviceStaticResource>();


    ThrowIfFailed(D3dResources::GetDevice()->CreateCommittedResource(
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
void TriangleRenderProxy::UploadDynamic(int game_frame)
{
    return;
}
void TriangleRenderProxy::UploadStatic()
{
    UINT8* pVertexDataBegin;
    CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
    ThrowIfFailed(device_static_resource->m_vertex_buffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
    memcpy(pVertexDataBegin, vertex.data(), vertex.size() * sizeof(Vertex));
    device_static_resource->m_vertex_buffer->Unmap(0, nullptr);
    return;
}

void TriangleRenderProxy::IASet(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list) const
{
    command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    command_list->IASetVertexBuffers(0, 1, &device_static_resource->m_vertex_buffer_view);
}
void TriangleRenderProxy::RSSet(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, const ViewInfo& view) const
{
    command_list->RSSetViewports(1, &view.m_viewport);
    command_list->RSSetScissorRects(1, &view.m_scissor_rect);
}
void TriangleRenderProxy::OMSet(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, const ViewInfo& view) const
{
    command_list->OMSetRenderTargets(1, &view.render_target_view, FALSE, &view.depth_stencil_view);
}
void TriangleRenderProxy::Draw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list) const
{
    command_list->DrawInstanced(vertex.size(), 1, 0, 0);
}

void TriangleRenderProxy::PopulateCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, const ViewInfo& view, int buffer_index)
{
    assert(device_static_resource != nullptr);
    assert(b_render_resources_inited);
    assert(shading_model);
    IASet(command_list);
    RSSet(command_list, view);
    OMSet(command_list, view);
    shading_model->PopulateCommandList(command_list, buffer_index, &view, this);
    return;
}