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
static void CheckShaderCompile(HRESULT result, Microsoft::WRL::ComPtr<ID3DBlob> error_blob)
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

void ScreenTriangleShadingModel::PopulateCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,int buffer_index, const ViewInfo* p_view, const RenderProxy* p_render_proxy)
{
	command_list->SetPipelineState(m_pipeline_state.Get());
	command_list->SetGraphicsRootSignature(m_root_signature[buffer_index].Get());
	SetRootSignatures(command_list, buffer_index, p_view, p_render_proxy);
	p_render_proxy->Draw(command_list);
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
	view_buffer.viewport_size = { int(p_view->m_viewport.Width),int(p_view->m_viewport.Height) };
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
	HRESULT result = D3DCompileFromFile(L"./shader/alpha_triangle/vertex_shader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", compileFlags, 0, &m_vertex_shader, &errorBlob);
	CheckShaderCompile(result, errorBlob);
	result = D3DCompileFromFile(L"./shader/alpha_triangle/pixel_shader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", compileFlags, 0, &m_pixel_shader, &errorBlob);
	CheckShaderCompile(result, errorBlob);
	result = D3DCompileFromFile(L"./shader/alpha_triangle/clear_perpixel_linked_list.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CSMain", "cs_5_0", compileFlags, 0, &m_clear_cs, &errorBlob);
	CheckShaderCompile(result, errorBlob);

	//init descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC start_offset_uav_desc = {};
	start_offset_uav_desc.NumDescriptors = D3dResources::SWAPCHAIN_BUFFERCOUNT * 3;
	start_offset_uav_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	start_offset_uav_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	m_per_pixel_linked_list_uavheap.Init(start_offset_uav_desc);
	

	////////////////////////////////////////////////// PerPixelLinkedList///////////////////////////////////////
	struct PerPixelLinkedList_Node
	{
		int primitive_id;
		int next;
	};

	//create resource

	auto startoffset_buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(MAX_SCREEN_SIZE *sizeof(int), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	auto linker_buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(MAX_SCREEN_SIZE * 8 * sizeof(PerPixelLinkedList_Node), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	auto counter_desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(int), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	for (int i = 0; i < D3dResources::SWAPCHAIN_BUFFERCOUNT; i++)
	{
		ThrowIfFailed(D3dResources::GetDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &counter_desc,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&m_linker_counter[i])));
		ThrowIfFailed(D3dResources::GetDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &startoffset_buffer_desc,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&m_start_offset_buffer[i])));
		ThrowIfFailed(D3dResources::GetDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &linker_buffer_desc,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&m_linker_buffer[i])));
	}

	//create uav

	D3D12_UNORDERED_ACCESS_VIEW_DESC startoffset_uav_desc;
	startoffset_uav_desc.Format = DXGI_FORMAT_R32_TYPELESS;
	startoffset_uav_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	startoffset_uav_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
	startoffset_uav_desc.Buffer.FirstElement = 0;
	startoffset_uav_desc.Buffer.NumElements = MAX_SCREEN_SIZE;
	startoffset_uav_desc.Buffer.CounterOffsetInBytes = 0;
	startoffset_uav_desc.Buffer.StructureByteStride = 0;

	D3D12_UNORDERED_ACCESS_VIEW_DESC linker_uav_desc;
	linker_uav_desc.Format = DXGI_FORMAT_UNKNOWN;
	linker_uav_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	linker_uav_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	linker_uav_desc.Buffer.FirstElement = 0;
	linker_uav_desc.Buffer.NumElements = MAX_SCREEN_SIZE * 8;
	linker_uav_desc.Buffer.CounterOffsetInBytes = 0;
	linker_uav_desc.Buffer.StructureByteStride = sizeof(PerPixelLinkedList_Node);

	D3D12_UNORDERED_ACCESS_VIEW_DESC linker_counter_desc;
	linker_counter_desc.Format = DXGI_FORMAT_R32_SINT;
	linker_counter_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	linker_counter_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	linker_counter_desc.Buffer.FirstElement = 0;
	linker_counter_desc.Buffer.NumElements = 1;
	linker_counter_desc.Buffer.CounterOffsetInBytes = 0;
	linker_counter_desc.Buffer.StructureByteStride = 0;

	for (int i = 0; i < D3dResources::SWAPCHAIN_BUFFERCOUNT; i++)
	{
		//start offset
		D3dResources::GetDevice()->CreateUnorderedAccessView(m_start_offset_buffer[i].Get(), nullptr, &startoffset_uav_desc, m_per_pixel_linked_list_uavheap[i*3+0]);
		//linker
		D3dResources::GetDevice()->CreateUnorderedAccessView(m_linker_buffer[i].Get(), m_linker_counter[i].Get(), &linker_uav_desc, m_per_pixel_linked_list_uavheap[i * 3 + 1]);
		//linker counter
		D3dResources::GetDevice()->CreateUnorderedAccessView(m_linker_counter[i].Get(), nullptr, &linker_counter_desc, m_per_pixel_linked_list_uavheap[i * 3 + 2]);
	}
}

void AlphaMeshShadingModel::InitRootSignature()
{
	CD3DX12_ROOT_PARAMETER rootParameters[3];
	rootParameters[ViewCBufferIndex].InitAsConstants(sizeof(ViewBuffer) / 4, 0, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[BatchCBufferIndex].InitAsConstants(sizeof(BatchBuffer) / 4, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	CD3DX12_DESCRIPTOR_RANGE DescRange[1];
	DescRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 3, 1);
	rootParameters[UAV_Index].InitAsDescriptorTable(1, &DescRange[0]);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	Microsoft::WRL::ComPtr<ID3DBlob> serialized_signature_desc;
	Microsoft::WRL::ComPtr<ID3DBlob> error;
	HRESULT result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized_signature_desc, &error);
	CheckShaderCompile(result, error);

	for (int i = 0; i < D3dResources::SWAPCHAIN_BUFFERCOUNT; i++)
	{
		ThrowIfFailed(D3dResources::GetDevice()->CreateRootSignature(0, serialized_signature_desc->GetBufferPointer(), serialized_signature_desc->GetBufferSize(), IID_PPV_ARGS(&m_root_signature[i])));
	}
}

void AlphaMeshShadingModel::ClearBuffer(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, int buffer_index)
{
	//todo clear perpixel linked list
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
void AlphaMeshShadingModel::InitPSO()
{
	BasicMeshShadingModel::InitPSO();

	D3D12_COMPUTE_PIPELINE_STATE_DESC pso_desc = {};
	pso_desc.pRootSignature = m_root_signature->Get();
	pso_desc.CS.pShaderBytecode = m_clear_cs->GetBufferPointer();
	pso_desc.CS.BytecodeLength = m_clear_cs->GetBufferSize();
	pso_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	ThrowIfFailed(D3dResources::GetDevice()->CreateComputePipelineState(&pso_desc, IID_PPV_ARGS(&m_clear_pso)));
}

void AlphaMeshShadingModel::CreatePerpixelLinkedListCall(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, int buffer_index, const ViewInfo* p_view, const RenderProxy* p_render_proxy)
{
	command_list->SetPipelineState(m_clear_pso.Get());
	command_list->SetComputeRootSignature(m_root_signature[buffer_index].Get());
	ID3D12DescriptorHeap* pHeaps[] = { m_per_pixel_linked_list_uavheap.GetHeap().Get() };
	command_list->SetDescriptorHeaps(1, pHeaps);
	command_list->SetComputeRootDescriptorTable(UAV_Index, m_per_pixel_linked_list_uavheap.GetGPU(buffer_index * 3));
	command_list->Dispatch(MAX_SCREEN_SIZE / 512, 1, 1);

	D3D12_RESOURCE_BARRIER uavBarrier = {};
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = m_start_offset_buffer->Get();
	command_list->ResourceBarrier(1, &uavBarrier);

	//create per-pixel linked list
	command_list->SetPipelineState(m_pipeline_state.Get());
	command_list->SetGraphicsRootSignature(m_root_signature[buffer_index].Get());
	ViewBuffer view_buffer;
	view_buffer.view_transform = p_view->view_matrix;
	view_buffer.project_transform = p_view->project_matrix;
	view_buffer.viewport_size = { int(p_view->m_viewport.Width),int(p_view->m_viewport.Height) };
	BatchBuffer batch_buffer;
	batch_buffer.world_transform = p_render_proxy->world_transform;
	command_list->SetGraphicsRoot32BitConstants(ViewCBufferIndex, sizeof(ViewBuffer) / 4, &view_buffer, 0);
	command_list->SetGraphicsRoot32BitConstants(BatchCBufferIndex, sizeof(BatchBuffer) / 4, &batch_buffer, 0);
	command_list->SetDescriptorHeaps(1, pHeaps);
	command_list->SetGraphicsRootDescriptorTable(UAV_Index, m_per_pixel_linked_list_uavheap.GetGPU(buffer_index * 3));

	return;
}

void AlphaMeshShadingModel::PopulateCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, int buffer_index, const ViewInfo* p_view, const RenderProxy* p_render_proxy)
{
	//command_list->OMSetRenderTargets(0, nullptr, FALSE, nullptr);
	CreatePerpixelLinkedListCall(command_list,buffer_index,p_view,p_render_proxy);
	p_render_proxy->Draw(command_list);

	//render perpixel linked list
	//command_list->OMSetRenderTargets(1, &p_view->render_target_view, FALSE, &p_view->depth_stencil_view);
	return;
}
