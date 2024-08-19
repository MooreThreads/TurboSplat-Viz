#include"shading_model.h"
#include"d3d_helper.h"
#include<d3dcompiler.h>
#include "d3dx12.h"
#include<assert.h>
#include"DirectXMath.h"
#include "view_info.h"
#include "render_proxy.h"
//#include <stdexcept>

ScreenTriangleShadingModel::ScreenTriangleShadingModel():ShadingModel()
{

}

void ScreenTriangleShadingModel::Init()
{
	InitShader();
	InitRootSignature();
	InitPSO();
}
void ScreenTriangleShadingModel::CheckShaderCompile(HRESULT result, Microsoft::WRL::ComPtr<ID3DBlob> error_blob)
{
	if (FAILED(result))
	{
		if (error_blob)
		{
			OutputDebugStringA((char*)error_blob->GetBufferPointer());
		}
	}
}
void ScreenTriangleShadingModel::InitShader()
{
#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT vs_result=D3DCompileFromFile(L"./shader/default/vertex_shader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", compileFlags, 0, &m_vertex_shader, &errorBlob);
	CheckShaderCompile(vs_result, errorBlob);
	HRESULT ps_result = D3DCompileFromFile(L"./shader/default/pixel_shader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", compileFlags, 0, &m_pixel_shader, &errorBlob);
	CheckShaderCompile(ps_result, errorBlob);
}
void ScreenTriangleShadingModel::InitRootSignature()
{
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	Microsoft::WRL::ComPtr<ID3DBlob> serialized_signature_desc;
	Microsoft::WRL::ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized_signature_desc, &error));

	for (int i = 0; i < D3dResources::SWAPCHAIN_BUFFERCOUNT; i++)
	{
		ThrowIfFailed(D3dResources::GetDevice()->CreateRootSignature(0, serialized_signature_desc->GetBufferPointer(), serialized_signature_desc->GetBufferSize(), IID_PPV_ARGS(&m_root_signature[i])));
	}
}

void ScreenTriangleShadingModel::InitPSO()
{
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = m_root_signature[0].Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vertex_shader.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_pixel_shader.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	ThrowIfFailed(D3dResources::GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipeline_state)));
}

void ScreenTriangleShadingModel::PopulateCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,int buffer_index)
{
	command_list->SetPipelineState(m_pipeline_state.Get());
	command_list->SetGraphicsRootSignature(m_root_signature[buffer_index].Get());
	return;
}

void ScreenTriangleShadingModel::SetRootSignatures(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, int buffer_index, const ViewInfo* p_view, const RenderProxy* p_render_proxy)
{

}



BasicMeshShadingModel::BasicMeshShadingModel():ScreenTriangleShadingModel()
{

}
void BasicMeshShadingModel::InitShader()
{
#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT vs_result = D3DCompileFromFile(L"./shader/triangle/vertex_shader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", compileFlags, 0, &m_vertex_shader, &errorBlob);
	CheckShaderCompile(vs_result, errorBlob);
	HRESULT ps_result = D3DCompileFromFile(L"./shader/triangle/pixel_shader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", compileFlags, 0, &m_pixel_shader, &errorBlob);
	CheckShaderCompile(ps_result, errorBlob);
}
void BasicMeshShadingModel::InitRootSignature()
{
	CD3DX12_ROOT_PARAMETER rootParameters[2];
	rootParameters[ViewCBufferIndex].InitAsConstants(sizeof(ViewBuffer) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[BatchCBufferIndex].InitAsConstants(sizeof(BatchBuffer) / 4, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	Microsoft::WRL::ComPtr<ID3DBlob> serialized_signature_desc;
	Microsoft::WRL::ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized_signature_desc, &error));

	for (int i = 0; i < D3dResources::SWAPCHAIN_BUFFERCOUNT; i++)
	{
		ThrowIfFailed(D3dResources::GetDevice()->CreateRootSignature(0, serialized_signature_desc->GetBufferPointer(), serialized_signature_desc->GetBufferSize(), IID_PPV_ARGS(&m_root_signature[i])));
	}
}

void BasicMeshShadingModel::SetRootSignatures(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, int buffer_index,const ViewInfo* p_view, const RenderProxy* p_render_proxy)
{
	//p.s. 
	//		CPU						HLSL
	// row major			-> column major
	// vec_x matmul mat_T	-> mat_T matmul vec_x
	// so we do not need to do anything
	ViewBuffer view_buffer;
	view_buffer.view_transform = p_view->view_matrix;
	view_buffer.project_transform = p_view->project_matrix;
	BatchBuffer batch_buffer;
	batch_buffer.world_transform = p_render_proxy->world_transform;

	command_list->SetGraphicsRoot32BitConstants(ViewCBufferIndex, sizeof(ViewBuffer) / 4, &view_buffer, 0);
	command_list->SetGraphicsRoot32BitConstants(BatchCBufferIndex, sizeof(BatchBuffer) / 4, &batch_buffer, 0);

	return;
}