#pragma once
#include<d3d.h>
#include <wrl.h>
#include<map>
#include<memory>
#include"DirectXMath.h"
#include "d3d12.h"

class ViewInfo;
class RenderProxy;
class ID3D12GraphicsCommandList;
class DefaultGraphicPipeline;
class StaticMeshGraphicPipeline;

class MeshGaussianClear;
class MeshGaussianClusterCulling;
class MeshGaussianFillData;
class MeshGaussianSort;
class MeshGaussianRaster;


namespace D3DHelper
{
	class StaticDescriptorStack;
	class Device;
}

class ShadingModel
{
public:
	virtual void RegisterDevice(std::shared_ptr<D3DHelper::Device> device)=0;
	virtual void PopulateCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
		D3DHelper::StaticDescriptorStack(&binded_heaps)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
		int buffer_index, const ViewInfo* p_view, const RenderProxy* p_render_proxy)=0;
};

class ScreenTriangleShadingModel:public ShadingModel
{
private:
	struct PilelineList {
		std::shared_ptr<DefaultGraphicPipeline> draw_triangle_pipeline;
	};
	std::map<D3DHelper::Device*, PilelineList> m_device_pipeline_list;
public:
	ScreenTriangleShadingModel();
	virtual void RegisterDevice(std::shared_ptr<D3DHelper::Device> device);
	virtual void PopulateCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
		D3DHelper::StaticDescriptorStack(&binded_heaps)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES], 
		int buffer_index, const ViewInfo* p_view, const RenderProxy* p_render_proxy);
	
};

class BasicMeshShadingModel:public ShadingModel
{
private:
	struct PilelineList {
		std::shared_ptr<StaticMeshGraphicPipeline> draw_triangle_pipeline;
	};
	std::map<D3DHelper::Device*, PilelineList> m_device_pipeline_list;

public:
	BasicMeshShadingModel();
	virtual void RegisterDevice(std::shared_ptr<D3DHelper::Device> device);
	virtual void PopulateCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
		D3DHelper::StaticDescriptorStack(&binded_heaps)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
		int buffer_index, const ViewInfo* p_view, const RenderProxy* p_render_proxy);
};

class GaussianSplattingShadingModel :public ShadingModel
{
private:
	struct PilelineList {
		std::shared_ptr < MeshGaussianClear> gs_clear_pipeline;
		std::shared_ptr <MeshGaussianClusterCulling> gs_cluster_culling_pipeline;
		std::shared_ptr < MeshGaussianFillData> gs_filldata_pipeline;
		std::shared_ptr < MeshGaussianSort> gs_sort_pipeline;
		std::shared_ptr <MeshGaussianRaster> draw_mesh_gs_pipeline;
	};
	std::map<D3DHelper::Device*, PilelineList> m_device_pipeline_list;

	struct IntermediateBuffer
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> cluster_counter_buffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> visible_cluster_buffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> point_counter_buffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> visible_point_buffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> visible_point_depth_buffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> indirect_arg_filldata_buffer;
		
	};
	const int MAX_CLUSTER_NUM = 1024 * 1024;
	const int MAX_POINTS_NUM = 50 * 1024 * 1024;
	std::map<D3DHelper::Device*, IntermediateBuffer> m_device_intermediate_buffer_list;


public:
	GaussianSplattingShadingModel();
	virtual void RegisterDevice(std::shared_ptr<D3DHelper::Device> device);
	virtual void PopulateCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
		D3DHelper::StaticDescriptorStack(&binded_heaps)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
		int buffer_index, const ViewInfo* p_view, const RenderProxy* p_render_proxy);
};

