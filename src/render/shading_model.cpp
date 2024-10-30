#include"shading_model.h"
#include"d3d_helper.h"
#include<d3dcompiler.h>
#include "d3dx12.h"
#include<assert.h>
#include"DirectXMath.h"
#include "view_info.h"
#include "render_proxy.h"
#include"device.h"
#include "mesh_gaussian_pipeline.h"

ScreenTriangleShadingModel::ScreenTriangleShadingModel():ShadingModel()
{

}
void ScreenTriangleShadingModel::RegisterDevice(std::shared_ptr<D3DHelper::Device> device)
{
	assert(m_device_pipeline_list.find(device.get()) == m_device_pipeline_list.end());
	PilelineList pipeline_list = {};
	pipeline_list.draw_triangle_pipeline = std::make_shared<DefaultGraphicPipeline>();
	pipeline_list.draw_triangle_pipeline->Init(device);
	m_device_pipeline_list[device.get()] = pipeline_list;
}
void ScreenTriangleShadingModel::PopulateCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
	D3DHelper::StaticDescriptorStack(&binded_heaps)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
	int buffer_index, const ViewInfo* p_view, const RenderProxy* p_render_proxy)
{
	assert(m_device_pipeline_list.find(p_render_proxy->m_device.get()) != m_device_pipeline_list.end());
	m_device_pipeline_list[p_render_proxy->m_device.get()].draw_triangle_pipeline->Draw(command_list, binded_heaps, buffer_index, p_view, p_render_proxy);
}


BasicMeshShadingModel::BasicMeshShadingModel():ShadingModel()
{

}

void BasicMeshShadingModel::RegisterDevice(std::shared_ptr<D3DHelper::Device> device)
{
	assert(m_device_pipeline_list.find(device.get()) == m_device_pipeline_list.end());
	PilelineList pipeline_list = {};
	pipeline_list.draw_triangle_pipeline = std::make_shared<StaticMeshGraphicPipeline>();
	pipeline_list.draw_triangle_pipeline->Init(device);
	m_device_pipeline_list[device.get()] = pipeline_list;
}

void BasicMeshShadingModel::PopulateCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
	D3DHelper::StaticDescriptorStack(&binded_heaps)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
	int buffer_index, const ViewInfo* p_view, const RenderProxy* p_render_proxy)
{
	assert(m_device_pipeline_list.find(p_render_proxy->m_device.get()) != m_device_pipeline_list.end());
	m_device_pipeline_list[p_render_proxy->m_device.get()].draw_triangle_pipeline->Draw(command_list, binded_heaps, buffer_index, p_view, p_render_proxy);
}

GaussianSplattingShadingModel::GaussianSplattingShadingModel() :ShadingModel()
{

}

void GaussianSplattingShadingModel::RegisterDevice(std::shared_ptr<D3DHelper::Device> device)
{
	assert(m_device_pipeline_list.find(device.get()) == m_device_pipeline_list.end());

	//init buffer
	IntermediateBuffer buffer;
	ThrowIfFailed(device->GetDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(int), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffer.cluster_counter_buffer)));
	ThrowIfFailed(device->GetDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(int)* MAX_CLUSTER_NUM, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffer.visible_cluster_buffer)));
	ThrowIfFailed(device->GetDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(int), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffer.point_counter_buffer)));
	ThrowIfFailed(device->GetDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(int) * MAX_POINTS_NUM, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffer.visible_point_buffer)));
	ThrowIfFailed(device->GetDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(int) * MAX_POINTS_NUM, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffer.visible_point_depth_buffer)));
	ThrowIfFailed(device->GetDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(MeshGaussianFillData::IndirectCommand), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffer.indirect_arg_filldata_buffer)));
	ThrowIfFailed(device->GetDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(MeshGaussianFillData::IndirectCommand), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS), D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffer.indirect_arg_sort_buffer)));
	buffer.cluster_counter_buffer->SetName(L"visible cluster counter");
	buffer.visible_cluster_buffer->SetName(L"visible cluster id");
	buffer.point_counter_buffer->SetName(L"visible point counter");
	buffer.visible_point_buffer->SetName(L"visible point id");
	buffer.visible_point_depth_buffer->SetName(L"visible point uint depth");
	buffer.indirect_arg_filldata_buffer->SetName(L"FillDataCS indirect args");
	buffer.indirect_arg_sort_buffer->SetName(L"Sort indirect args");
	m_device_intermediate_buffer_list[device.get()] = buffer;


	PilelineList pipeline_list = {};
	pipeline_list.gs_clear_pipeline= std::make_shared<MeshGaussianClear>();
	pipeline_list.gs_clear_pipeline->Init(device, buffer.cluster_counter_buffer, buffer.point_counter_buffer, 
		buffer.indirect_arg_filldata_buffer, buffer.indirect_arg_sort_buffer);
	pipeline_list.gs_cluster_culling_pipeline = std::make_shared<MeshGaussianClusterCulling>();
	pipeline_list.gs_cluster_culling_pipeline->Init(device, buffer.cluster_counter_buffer, buffer.visible_cluster_buffer, buffer.indirect_arg_filldata_buffer, MAX_CLUSTER_NUM);
	pipeline_list.gs_filldata_pipeline = std::make_shared<MeshGaussianFillData>();
	pipeline_list.gs_filldata_pipeline->Init(device, buffer.cluster_counter_buffer, buffer.visible_cluster_buffer, buffer.visible_point_buffer,
		buffer.visible_point_depth_buffer, buffer.point_counter_buffer,buffer.indirect_arg_sort_buffer, MAX_CLUSTER_NUM, MAX_POINTS_NUM);
	pipeline_list.gs_sort_pipeline= std::make_shared<MeshGaussianSort>();
	pipeline_list.gs_sort_pipeline->Init(device);
	pipeline_list.draw_mesh_gs_pipeline = std::make_shared<MeshGaussianRaster>();
	pipeline_list.draw_mesh_gs_pipeline->Init(device, buffer.point_counter_buffer, buffer.visible_point_buffer, MAX_POINTS_NUM);
	m_device_pipeline_list[device.get()] = pipeline_list;


}

void GaussianSplattingShadingModel::PopulateCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
	D3DHelper::StaticDescriptorStack(&binded_heaps)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
	int buffer_index, const ViewInfo* p_view, const RenderProxy* p_render_proxy)
{
	assert(m_device_pipeline_list.find(p_render_proxy->m_device.get()) != m_device_pipeline_list.end());
	const auto& pipeline_list = m_device_pipeline_list[p_render_proxy->m_device.get()];
	const auto& buffer=m_device_intermediate_buffer_list[p_render_proxy->m_device.get()];
	pipeline_list.gs_clear_pipeline->Dispatch(command_list, binded_heaps, buffer_index, p_view, p_render_proxy);
	{
		D3D12_RESOURCE_BARRIER uav_barriers[] = {
				CD3DX12_RESOURCE_BARRIER::UAV(buffer.point_counter_buffer.Get()),
				CD3DX12_RESOURCE_BARRIER::UAV(buffer.cluster_counter_buffer.Get()),
				CD3DX12_RESOURCE_BARRIER::UAV(buffer.indirect_arg_filldata_buffer.Get()),
				CD3DX12_RESOURCE_BARRIER::UAV(buffer.indirect_arg_sort_buffer.Get())
		};
		command_list->ResourceBarrier(_countof(uav_barriers), uav_barriers);
	}
	pipeline_list.gs_cluster_culling_pipeline->Dispatch(command_list, binded_heaps, buffer_index, p_view, p_render_proxy);
	{
		D3D12_RESOURCE_BARRIER uav_barriers[] = {
			CD3DX12_RESOURCE_BARRIER::Transition(buffer.cluster_counter_buffer.Get(),D3D12_RESOURCE_STATE_COMMON,D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
			CD3DX12_RESOURCE_BARRIER::Transition(buffer.visible_cluster_buffer.Get(),D3D12_RESOURCE_STATE_COMMON,D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
			CD3DX12_RESOURCE_BARRIER::Transition(buffer.indirect_arg_filldata_buffer.Get(),D3D12_RESOURCE_STATE_COMMON,D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT)
		};
		command_list->ResourceBarrier(_countof(uav_barriers), uav_barriers);
	}
	//pipeline_list.gs_filldata_pipeline->Dispatch(command_list, binded_heaps, buffer_index, p_view, p_render_proxy);
	pipeline_list.gs_filldata_pipeline->DispatchIndirect(command_list, buffer.indirect_arg_filldata_buffer,
		binded_heaps, buffer_index, p_view, p_render_proxy);
	{
		D3D12_RESOURCE_BARRIER uav_barriers[] = {
			CD3DX12_RESOURCE_BARRIER::Transition(buffer.point_counter_buffer.Get(),D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE| D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER),
			CD3DX12_RESOURCE_BARRIER::Transition(buffer.indirect_arg_sort_buffer.Get(),D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER),
			CD3DX12_RESOURCE_BARRIER::UAV(buffer.visible_point_depth_buffer.Get()),
			CD3DX12_RESOURCE_BARRIER::UAV(buffer.visible_point_buffer.Get())
		};
		command_list->ResourceBarrier(_countof(uav_barriers), uav_barriers);
	}
	pipeline_list.gs_sort_pipeline->Dispatch(command_list, 987327,
		buffer.point_counter_buffer, buffer.indirect_arg_sort_buffer,
		buffer.visible_point_depth_buffer, buffer.visible_point_buffer);
	pipeline_list.draw_mesh_gs_pipeline->Draw(command_list, binded_heaps, buffer_index, p_view, p_render_proxy);
}
