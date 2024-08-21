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

D3D12_DEPTH_STENCIL_DESC ScreenTriangleShadingModel::GetDepthStencilDesc()
{
	D3D12_DEPTH_STENCIL_DESC depth_stencil_desc;
	depth_stencil_desc.DepthEnable = true;
	depth_stencil_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depth_stencil_desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depth_stencil_desc.StencilEnable = false;

	return depth_stencil_desc;
}

D3D12_BLEND_DESC ScreenTriangleShadingModel::GetBlendDesc()
{
	D3D12_BLEND_DESC blend_desc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	return blend_desc;

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
	psoDesc.BlendState = GetBlendDesc();
	psoDesc.DepthStencilState = GetDepthStencilDesc();
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
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


AlphaMeshShadingModel::AlphaMeshShadingModel():BasicMeshShadingModel()
{

}

void AlphaMeshShadingModel::InitShader()
{
#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT vs_result = D3DCompileFromFile(L"./shader/alpha_triangle/vertex_shader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", compileFlags, 0, &m_vertex_shader, &errorBlob);
	CheckShaderCompile(vs_result, errorBlob);
	HRESULT ps_result = D3DCompileFromFile(L"./shader/alpha_triangle/pixel_shader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", compileFlags, 0, &m_pixel_shader, &errorBlob);
	CheckShaderCompile(ps_result, errorBlob);

	//create UAV
	auto buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(1920*2*1080*2*sizeof(int), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	D3D12_DESCRIPTOR_HEAP_DESC start_offset_uav_desc = {}; // Note: DepthStencil View requires storage in a heap even if we are going to use only 1 view
	start_offset_uav_desc.NumDescriptors = D3dResources::SWAPCHAIN_BUFFERCOUNT;
	start_offset_uav_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	start_offset_uav_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	m_start_offset_uav.Init(start_offset_uav_desc);
	for (int i = 0; i < D3dResources::SWAPCHAIN_BUFFERCOUNT; i++)
	{
		D3dResources::GetDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &buffer_desc,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&m_start_offset_buffer[i]));
		D3D12_UNORDERED_ACCESS_VIEW_DESC view_desc;
		view_desc.Format = DXGI_FORMAT_UNKNOWN;
		view_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		view_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		view_desc.Buffer.FirstElement = 0;
		view_desc.Buffer.NumElements = 1920 * 2 * 1080 * 2;
		view_desc.Buffer.StructureByteStride = sizeof(int);
		view_desc.Buffer.CounterOffsetInBytes = 0;
		D3dResources::GetDevice()->CreateUnorderedAccessView(m_start_offset_buffer[i].Get(), nullptr, &view_desc, m_start_offset_uav[i]);
	}
}

void AlphaMeshShadingModel::InitRootSignature()
{
	CD3DX12_ROOT_PARAMETER rootParameters[3];
	rootParameters[ViewCBufferIndex].InitAsConstants(sizeof(ViewBuffer) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[BatchCBufferIndex].InitAsConstants(sizeof(BatchBuffer) / 4, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[UAV_Index].InitAsUnorderedAccessView(1, 0, D3D12_SHADER_VISIBILITY_PIXEL);

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

void AlphaMeshShadingModel::SetRootSignatures(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, int buffer_index, const ViewInfo* p_view, const RenderProxy* p_render_proxy)
{
	BasicMeshShadingModel::SetRootSignatures(command_list, buffer_index, p_view, p_render_proxy);
	command_list->SetGraphicsRootUnorderedAccessView(UAV_Index, m_start_offset_buffer[buffer_index]->GetGPUVirtualAddress());

	return;
}

D3D12_DEPTH_STENCIL_DESC AlphaMeshShadingModel::GetDepthStencilDesc()
{
	//disable Depth Wirte
	//enable Depth Test
	D3D12_DEPTH_STENCIL_DESC depth_stencil_desc;
	depth_stencil_desc.DepthEnable = true;
	depth_stencil_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depth_stencil_desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depth_stencil_desc.StencilEnable = false;

	return depth_stencil_desc;
}
D3D12_BLEND_DESC AlphaMeshShadingModel::GetBlendDesc()
{
	D3D12_BLEND_DESC blend_desc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	blend_desc.RenderTarget[0].BlendEnable = true;
	blend_desc.RenderTarget[0].LogicOpEnable = FALSE;
	blend_desc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_COLOR;
	blend_desc.RenderTarget[0].DestBlend = D3D12_BLEND_DEST_COLOR;
	blend_desc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blend_desc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
	blend_desc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
	blend_desc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;

	return blend_desc;
}