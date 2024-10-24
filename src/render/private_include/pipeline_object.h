#pragma once
#include <wrl.h>
#include <vector>
#include <string>
#include "d3d12.h"
#include "dxcapi.h"
#include "device.h"
#include "descriptor_heap.h"
#include "view_info.h"

class RenderProxy;

void CompileShaingModel6(LPCWSTR* pszArgs, int args_num, LPCWSTR source_file, Microsoft::WRL::ComPtr<IDxcBlob>& pShader);

class Pipeline
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


class ComputePipeline:public Pipeline
{
protected:
	Microsoft::WRL::ComPtr<ID3DBlob> m_compute_shader;
	std::wstring m_shader_dir;
	std::wstring m_shader_name;
	static void CompileComputeShader(std::wstring shader_dir, std::wstring shader_name, std::wstring entry, Microsoft::WRL::ComPtr<ID3DBlob>* out_shader);
	static void CompileComputeShader(std::wstring shader_dir, std::wstring shader_name, std::wstring entry,const std::vector<std::wstring>& add_arg, Microsoft::WRL::ComPtr<ID3DBlob>* out_shader);
	virtual void InitShaders();
	virtual void InitPSO();
	virtual void CommitDescriptors(D3DHelper::StaticDescriptorStack(&param_stacks)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES]);

public:
	ComputePipeline();
	virtual void Init(std::shared_ptr<D3DHelper::Device> device);
	virtual void Draw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
		D3DHelper::StaticDescriptorStack(&param_stacks)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
		int buffer_index,
		const ViewInfo* p_view,
		const RenderProxy* proxy);
	virtual void Dispatch(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
		D3DHelper::StaticDescriptorStack(&param_stacks)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
		int buffer_index,
		const ViewInfo* p_view,
		const RenderProxy* proxy)=0;
};

class DefaultGraphicPipeline: public Pipeline
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

class PerPixelLinkedListPipeline :public Pipeline
{

};