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
class MeshGaussianPipeline;
class MeshGaussianClusterCulling;

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
		std::shared_ptr<MeshGaussianClusterCulling> gs_cluster_culling_pipeline;
		std::shared_ptr<MeshGaussianPipeline> draw_mesh_gs_pipeline;
	};
	std::map<D3DHelper::Device*, PilelineList> m_device_pipeline_list;

public:
	GaussianSplattingShadingModel();
	virtual void RegisterDevice(std::shared_ptr<D3DHelper::Device> device);
	virtual void PopulateCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
		D3DHelper::StaticDescriptorStack(&binded_heaps)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
		int buffer_index, const ViewInfo* p_view, const RenderProxy* p_render_proxy);
};

