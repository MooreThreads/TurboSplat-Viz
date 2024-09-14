#pragma once
#include <wrl.h>
#include <vector>
#include "d3d12.h"
#include "device.h"
#include "descriptor_heap.h"
#include "view_info.h"

class RenderProxy;

class GraphicPileine
{
protected:
	virtual void InitShaders()=0;
	virtual void InitRootSignature()=0;
	virtual void InitPSO()=0;
	virtual void InitResources()=0;

	virtual void SetRootSignature(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, const ViewInfo* p_view, const RenderProxy* proxy,
		D3D12_GPU_DESCRIPTOR_HANDLE stack_bottom[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES])=0;
	virtual void CommitDescriptors(D3DHelper::StaticDescriptorStack(&param_stacks)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES])=0;

	std::shared_ptr<D3DHelper::Device> m_device;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_root_signature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipeline_state;
	D3DHelper::StaticDescriptorHeap m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

public:
	virtual void Init(std::shared_ptr<D3DHelper::Device> device)=0;
	virtual void Draw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
		D3DHelper::StaticDescriptorStack(&param_stacks)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
		int buffer_index,
		const ViewInfo* p_view,
		const RenderProxy* proxy)=0;
};

class MSGraphicPileine
{

};

class ComputePipeline
{

};

class DefaultGraphicPipeline: public GraphicPileine
{
protected:
	Microsoft::WRL::ComPtr<ID3DBlob> m_vertex_shader;
	Microsoft::WRL::ComPtr<ID3DBlob> m_pixel_shader;

	virtual void InitShaders();
	virtual void InitRootSignature();
	virtual void InitPSO();
	virtual void InitResources();

	virtual void SetRootSignature(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, const ViewInfo* p_view, const RenderProxy* proxy, 
		D3D12_GPU_DESCRIPTOR_HANDLE* stack_bottom);
	virtual void CommitDescriptors(D3DHelper::StaticDescriptorStack(&param_stacks)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES]);

public:
	DefaultGraphicPipeline();
	virtual void Init(std::shared_ptr<D3DHelper::Device> device);
	virtual void Draw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, 
		D3DHelper::StaticDescriptorStack (& param_stacks)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES], 
		int buffer_index,
		const ViewInfo* p_view,
		const RenderProxy* proxy);
};

class StaticMeshGraphicPipeline : public DefaultGraphicPipeline
{

protected:
	struct ViewBuffer
	{
		DirectX::XMMATRIX view_transform;
		DirectX::XMMATRIX project_transform;
		DirectX::XMINT2 viewport_size;
	};
	struct BatchBuffer
	{
		DirectX::XMMATRIX world_transform;
	};

	virtual void InitShaders();
	virtual void InitRootSignature();

	virtual void SetRootSignature(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, const ViewInfo* p_view, const RenderProxy* proxy, 
		D3D12_GPU_DESCRIPTOR_HANDLE* stack_bottom);

};


class MeshGaussianPipeline:public DefaultGraphicPipeline
{
protected:
	struct ViewBuffer
	{
		DirectX::XMMATRIX view_transform;
		DirectX::XMMATRIX project_transform;
		DirectX::XMINT2 viewport_size;
		DirectX::XMFLOAT2 focal;
	};
	struct BatchBuffer
	{
		DirectX::XMMATRIX world_transform;
	};

	Microsoft::WRL::ComPtr<ID3D12Resource> gaussian_texture_buffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> gaussian_texture_upload_buffer;
	std::vector<std::vector<float>> cpu_gaussian_texture_buffer;
	bool pipeline_data_dirty;

	virtual void InitShaders();
	virtual void InitRootSignature();
	virtual void InitPSO();
	virtual void InitResources();

	virtual void SetRootSignature(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, const ViewInfo* p_view, const RenderProxy* proxy, 
		D3D12_GPU_DESCRIPTOR_HANDLE* stack_bottom);
	virtual void CommitDescriptors(D3DHelper::StaticDescriptorStack(&param_stacks)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES]);
	virtual void UpdatePipelineData(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list);

public:
	const DirectX::XMINT2 GAUSSIAN_TEXTURE_SIZE = { 128, 128 };
	const int GAUSSIAN_TEXTURE_LOD = 7;

	MeshGaussianPipeline();
	virtual void Draw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
		D3DHelper::StaticDescriptorStack(&param_stacks)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
		int buffer_index,
		const ViewInfo* p_view,
		const RenderProxy* proxy);
};

class PerPixelLinkedListPipeline :public GraphicPileine
{

};