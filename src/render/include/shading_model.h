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
	virtual void PopulateCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, int buffer_index)=0;
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
	void CheckShaderCompile(HRESULT result, Microsoft::WRL::ComPtr<ID3DBlob> error_blob);
	virtual void InitPSO();
public:
	ScreenTriangleShadingModel();
	virtual void Init();
	virtual void PopulateCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,int buffer_index);
	virtual void SetRootSignatures(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, int buffer_index,const ViewInfo* p_view,const RenderProxy* p_render_proxy);
};

class BasicMeshShadingModel:public ScreenTriangleShadingModel
{
public:
	struct ViewBuffer
	{
		DirectX::XMMATRIX view_transform;
		DirectX::XMMATRIX project_transform;
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
public:
	BasicMeshShadingModel();
	virtual void SetRootSignatures(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, int buffer_index,const ViewInfo* p_view,const RenderProxy* p_render_proxy);
};