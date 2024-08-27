#pragma once
#include<d3d.h>
#include"d3d_resources.h"
#include"DirectXMath.h"

class ViewInfo;
class RenderProxy;

class ShadingModel
{
public:
	virtual void Init()=0;
	virtual void PopulateCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, int buffer_index, const ViewInfo* p_view, const RenderProxy* p_render_proxy)=0;
protected:
	virtual void SetRootSignatures(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, int buffer_index,const ViewInfo* p_view, const RenderProxy* p_render_proxy)=0;
};

class ScreenTriangleShadingModel:public ShadingModel
{
protected:
	Microsoft::WRL::ComPtr<ID3DBlob> m_vertex_shader;
	Microsoft::WRL::ComPtr<ID3DBlob> m_pixel_shader;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_root_signature[D3dResources::SWAPCHAIN_BUFFERCOUNT];
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipeline_state;

	virtual void InitShader();
	virtual void InitRootSignature();
	virtual D3D12_DEPTH_STENCIL_DESC GetDepthStencilDesc();
	virtual D3D12_BLEND_DESC GetBlendDesc();
	virtual void InitPSO();
	virtual void SetRootSignatures(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, int buffer_index, const ViewInfo* p_view, const RenderProxy* p_render_proxy);
public:
	ScreenTriangleShadingModel();
	virtual void Init();
	virtual void PopulateCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,int buffer_index, const ViewInfo* p_view, const RenderProxy* p_render_proxy);
	
};

class BasicMeshShadingModel:public ScreenTriangleShadingModel
{
public:
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
	const int ViewCBufferIndex = 0;
	const int BatchCBufferIndex = 1;
protected:

	virtual void InitShader();
	virtual void InitRootSignature();
	virtual void SetRootSignatures(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, int buffer_index, const ViewInfo* p_view, const RenderProxy* p_render_proxy);
public:
	BasicMeshShadingModel();
	
};

class AlphaMeshShadingModel :public BasicMeshShadingModel
{
protected:
	Microsoft::WRL::ComPtr<ID3DBlob> m_clear_cs;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_clear_pso;
	Microsoft::WRL::ComPtr <ID3D12Resource> m_start_offset_buffer[D3dResources::SWAPCHAIN_BUFFERCOUNT];
	Microsoft::WRL::ComPtr <ID3D12Resource> m_linker_buffer[D3dResources::SWAPCHAIN_BUFFERCOUNT];
	Microsoft::WRL::ComPtr <ID3D12Resource> m_linker_counter[D3dResources::SWAPCHAIN_BUFFERCOUNT];
	D3dDescriptorHeapHelper m_per_pixel_linked_list_uavheap;
	virtual void InitShader();
	virtual void InitRootSignature();
	virtual D3D12_DEPTH_STENCIL_DESC GetDepthStencilDesc();
	virtual D3D12_BLEND_DESC GetBlendDesc();
	virtual void ClearBuffer(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, int buffer_index);
	virtual void InitPSO();

	virtual void SetRootSignatures(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, int buffer_index, const ViewInfo* p_view, const RenderProxy* p_render_proxy) { return; }
	virtual void CreatePerpixelLinkedListCall(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, int buffer_index, const ViewInfo* p_view, const RenderProxy* p_render_proxy);
public:
	const int UAV_Index = 2;
	const int MAX_SCREEN_SIZE = 1920 * 1080;
	AlphaMeshShadingModel();
	
	virtual void PopulateCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, int buffer_index, const ViewInfo* p_view, const RenderProxy* p_render_proxy);
};

class GaussianSplattingShadingModel :public ShadingModel
{
public:
	struct PPLL_D3DFrameResource
	{
		Microsoft::WRL::ComPtr <ID3D12Resource> start_offset_buffer;
		Microsoft::WRL::ComPtr <ID3D12Resource> node_buffer;
		Microsoft::WRL::ComPtr <ID3D12Resource> node_counter;
		Microsoft::WRL::ComPtr <ID3D12Resource> target_buffer;
	};
	struct ViewBuffer
	{
		DirectX::XMMATRIX view_transform;
		DirectX::XMMATRIX project_transform;
		DirectX::XMINT2 viewport_size;
		DirectX::XMFLOAT2 focal_xy;
	};
	struct BatchBuffer
	{
		DirectX::XMMATRIX world_transform;
	};
	const int ViewCBufferIndex = 0;
	const int BatchCBufferIndex = 1;
	struct PerPixelLinkedList_Node
	{
		int primitive_id;
		int next;
	};
protected:
	Microsoft::WRL::ComPtr<ID3DBlob> m_clear_ppll_cs;
	Microsoft::WRL::ComPtr<ID3DBlob> m_create_ppll_ms;
	Microsoft::WRL::ComPtr<ID3DBlob> m_create_ppll_ps;
	Microsoft::WRL::ComPtr<ID3DBlob> m_render_ppll_cs;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_clear_ppll_pso;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_create_ppll_pso;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_root_signature_create_ppll;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_render_ppll_pso;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_root_signature_render_ppll;

	PPLL_D3DFrameResource m_frame_resources;
	D3dDescriptorHeapHelper descriptor_heap;

	virtual void SetRootSignatures(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, int buffer_index, const ViewInfo* p_view, const RenderProxy* p_render_proxy) {};
	void InitResources();
	void InitShaders();
	void InitPSO();
	void InitRootSignature();
public:
	GaussianSplattingShadingModel();
	const DirectX::XMINT2 MAX_SCREEN_SIZE = {1920, 1080};
	virtual void Init();
	virtual void PopulateCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, int buffer_index, const ViewInfo* p_view, const RenderProxy* p_render_proxy) ;
};