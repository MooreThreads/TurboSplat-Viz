#include"mesh_gaussian_pipeline.h"
#include "d3d_helper.h"
#include "d3d12shader.h"
#include "dxcapi.h"
#include "render_proxy.h"
#include <assert.h>
#include <cmath>


MeshGaussianClear::MeshGaussianClear() :ComputePipeline()
{
	m_shader_dir = L"./shader/ms_gaussian_splatting/";
	m_shader_name = L"ClearCS";
}
void MeshGaussianClear::InitResources() {  }
void MeshGaussianClear::SetRootSignature(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, const ViewInfo* p_view, const RenderProxy* proxy,
	D3D12_GPU_DESCRIPTOR_HANDLE* stack_bottom)
{
	command_list->SetComputeRootSignature(m_root_signature.Get());
	command_list->SetComputeRootDescriptorTable(0, stack_bottom[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]);
}
void MeshGaussianClear::InitRootSignature()
{
	CD3DX12_ROOT_PARAMETER rootParameters[1];
	CD3DX12_DESCRIPTOR_RANGE DescRange[1];
	DescRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 2, 0);//gaussian_clusters pints
	rootParameters[0].InitAsDescriptorTable(_countof(DescRange), DescRange, D3D12_SHADER_VISIBILITY_ALL);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	Microsoft::WRL::ComPtr<ID3DBlob> serialized_signature_desc;
	Microsoft::WRL::ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized_signature_desc, &error));
	ThrowIfFailed(m_device->GetDevice()->CreateRootSignature(0, serialized_signature_desc->GetBufferPointer(), serialized_signature_desc->GetBufferSize(), IID_PPV_ARGS(&m_root_signature)));
}
void MeshGaussianClear::Init(std::shared_ptr<D3DHelper::Device> device,
	Microsoft::WRL::ComPtr<ID3D12Resource> out_visible_cluster_counter_buffer,
	Microsoft::WRL::ComPtr<ID3D12Resource> out_visible_point_counter_buffer)
{
	ComputePipeline::Init(device);


	D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};
	heap_desc.NumDescriptors = 2;
	heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].Init(heap_desc, m_device);

	D3D12_UNORDERED_ACCESS_VIEW_DESC counter_uav_desc = {};
	counter_uav_desc.Format = DXGI_FORMAT_R32_TYPELESS;
	counter_uav_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	counter_uav_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
	counter_uav_desc.Buffer.CounterOffsetInBytes = 0;
	counter_uav_desc.Buffer.FirstElement = 0;
	counter_uav_desc.Buffer.NumElements = 1;
	counter_uav_desc.Buffer.StructureByteStride = 0;
	m_device->GetDevice()->CreateUnorderedAccessView(out_visible_cluster_counter_buffer.Get(), nullptr, &counter_uav_desc,
		m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV][0]);
	m_device->GetDevice()->CreateUnorderedAccessView(out_visible_point_counter_buffer.Get(), nullptr, &counter_uav_desc,
		m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV][1]);

}

void MeshGaussianClear::Dispatch(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
	D3DHelper::StaticDescriptorStack(&param_stacks)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
	int buffer_index,
	const ViewInfo* p_view,
	const RenderProxy* proxy)
{
	D3D12_GPU_DESCRIPTOR_HANDLE stack_bottom[2]{ param_stacks[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].GetTopGPU(), D3D12_GPU_DESCRIPTOR_HANDLE() };
	SetRootSignature(command_list, p_view, proxy, stack_bottom);
	CommitDescriptors(param_stacks);
	command_list->SetPipelineState(m_pipeline_state.Get());
	command_list->Dispatch(1, 1, 1);
}







MeshGaussianClusterCulling::MeshGaussianClusterCulling() :ComputePipeline()
{
	m_shader_dir = L"./shader/ms_gaussian_splatting/";
	m_shader_name = L"ClusterCullCS";
}

void MeshGaussianClusterCulling::Init(std::shared_ptr<D3DHelper::Device> device, Microsoft::WRL::ComPtr<ID3D12Resource> out_counter_buffer,
	Microsoft::WRL::ComPtr<ID3D12Resource> out_visible_cluster_buffer, const int MAX_CLUSTER_NUM)
{
	ComputePipeline::Init(device);

	this->MAX_CLUSTER_NUM = MAX_CLUSTER_NUM;

	D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};
	heap_desc.NumDescriptors = 2;
	heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].Init(heap_desc, m_device);

	D3D12_UNORDERED_ACCESS_VIEW_DESC counter_uav_desc = {};
	counter_uav_desc.Format = DXGI_FORMAT_R32_TYPELESS;
	counter_uav_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	counter_uav_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
	counter_uav_desc.Buffer.CounterOffsetInBytes = 0;
	counter_uav_desc.Buffer.FirstElement = 0;
	counter_uav_desc.Buffer.NumElements = 1;
	counter_uav_desc.Buffer.StructureByteStride = 0;
	m_device->GetDevice()->CreateUnorderedAccessView(out_counter_buffer.Get(), nullptr, &counter_uav_desc,
		m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV][0]);

	D3D12_UNORDERED_ACCESS_VIEW_DESC visible_cluster_uav_desc = {};
	visible_cluster_uav_desc.Format = DXGI_FORMAT_R32_SINT;
	visible_cluster_uav_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	visible_cluster_uav_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	visible_cluster_uav_desc.Buffer.CounterOffsetInBytes = 0;
	visible_cluster_uav_desc.Buffer.FirstElement = 0;
	visible_cluster_uav_desc.Buffer.NumElements = MAX_CLUSTER_NUM;
	visible_cluster_uav_desc.Buffer.StructureByteStride = 0;
	m_device->GetDevice()->CreateUnorderedAccessView(out_visible_cluster_buffer.Get(), nullptr, &visible_cluster_uav_desc,
		m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV][1]);
}

void MeshGaussianClusterCulling::InitResources()
{

	return;
}



void MeshGaussianClusterCulling::InitRootSignature()
{
	CD3DX12_ROOT_PARAMETER rootParameters[3];
	rootParameters[0].InitAsConstants(sizeof(float) * 4 * 6 / 4, 0, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[1].InitAsConstants(sizeof(int) / 4, 1, 0, D3D12_SHADER_VISIBILITY_ALL);
	CD3DX12_DESCRIPTOR_RANGE DescRange[2];
	DescRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0);//gaussian_clusters pints
	DescRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 2, 0);//visible_clusters visible_clusters_num
	rootParameters[2].InitAsDescriptorTable(_countof(DescRange), DescRange, D3D12_SHADER_VISIBILITY_ALL);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	Microsoft::WRL::ComPtr<ID3DBlob> serialized_signature_desc;
	Microsoft::WRL::ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized_signature_desc, &error));
	ThrowIfFailed(m_device->GetDevice()->CreateRootSignature(0, serialized_signature_desc->GetBufferPointer(), serialized_signature_desc->GetBufferSize(), IID_PPV_ARGS(&m_root_signature)));
}

void MeshGaussianClusterCulling::SetRootSignature(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, const ViewInfo* p_view, const RenderProxy* proxy,
	D3D12_GPU_DESCRIPTOR_HANDLE* stack_bottom)
{
	command_list->SetComputeRootSignature(m_root_signature.Get());

	ViewBuffer view_buffer;
	for(int i=0;i<6;i++)
		view_buffer.plane[i]=p_view->frustum_plane[i];
	BatchBuffer batch_buffer;
	batch_buffer.clusters_num = proxy->GetClusterCountPerInstance();

	command_list->SetComputeRoot32BitConstants(0, sizeof(ViewBuffer) / 4, &view_buffer, 0);
	command_list->SetComputeRoot32BitConstants(1, sizeof(BatchBuffer) / 4, &batch_buffer, 0);
	command_list->SetComputeRootDescriptorTable(2, stack_bottom[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]);

	auto gaussian_render_proxy = dynamic_cast<const GaussianRenderProxy*>(proxy);
	assert(gaussian_render_proxy);

}

void MeshGaussianClusterCulling::Dispatch(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
	D3DHelper::StaticDescriptorStack(&param_stacks)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
	int buffer_index,
	const ViewInfo* p_view,
	const RenderProxy* proxy)
{
	auto gaussian_render_proxy = dynamic_cast<const GaussianRenderProxy*>(proxy);
	assert(gaussian_render_proxy);
	assert(MAX_CLUSTER_NUM > proxy->GetClusterCountPerInstance());

	
	D3D12_GPU_DESCRIPTOR_HANDLE stack_bottom[2]{ param_stacks[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].GetTopGPU(), D3D12_GPU_DESCRIPTOR_HANDLE() };
	SetRootSignature(command_list, p_view, proxy, stack_bottom);
	gaussian_render_proxy->CommitParams(command_list, param_stacks);
	CommitDescriptors(param_stacks);
	command_list->SetPipelineState(m_pipeline_state.Get());
	command_list->Dispatch(std::ceil(proxy->GetClusterCountPerInstance() / 256.0f), 1, 1);
}



MeshGaussianFillData::MeshGaussianFillData() :ComputePipeline(), MAX_CLUSTER_NUM(0), MAX_POINTS_NUM(0)
{
	m_shader_dir = L"./shader/ms_gaussian_splatting/";
	m_shader_name = L"FillDataCS";
}
void MeshGaussianFillData::Init(std::shared_ptr<D3DHelper::Device> device,
	Microsoft::WRL::ComPtr<ID3D12Resource> in_visible_cluster_counter_buffer,
	Microsoft::WRL::ComPtr<ID3D12Resource> in_visible_cluster_buffer,
	Microsoft::WRL::ComPtr<ID3D12Resource> out_visible_point_buffer,
	Microsoft::WRL::ComPtr<ID3D12Resource> out_visible_depth_buffer,
	Microsoft::WRL::ComPtr<ID3D12Resource> out_visible_point_counter_buffer,
	const int MAX_CLUSTER_NUM, const int MAX_POINTS_NUM)
{
	ComputePipeline::Init(device);
	this->MAX_CLUSTER_NUM = MAX_CLUSTER_NUM;
	this->MAX_POINTS_NUM = MAX_POINTS_NUM;


	D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};
	heap_desc.NumDescriptors = 5;
	heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].Init(heap_desc, m_device);

	//SRV
	D3D12_SHADER_RESOURCE_VIEW_DESC counter_srv_desc;
	counter_srv_desc.Format = DXGI_FORMAT_R32_TYPELESS;
	counter_srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	counter_srv_desc.Buffer.FirstElement = 0;
	counter_srv_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
	counter_srv_desc.Buffer.NumElements = 1;
	counter_srv_desc.Buffer.StructureByteStride = 0;
	counter_srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	m_device->GetDevice()->CreateShaderResourceView(in_visible_cluster_counter_buffer.Get(), &counter_srv_desc,
		m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV][0]);

	//init srv: visible_cluster
	D3D12_SHADER_RESOURCE_VIEW_DESC visible_cluster_srv_desc;
	visible_cluster_srv_desc.Format = DXGI_FORMAT_R32_SINT;
	visible_cluster_srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	visible_cluster_srv_desc.Buffer.FirstElement = 0;
	visible_cluster_srv_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	visible_cluster_srv_desc.Buffer.NumElements = MAX_CLUSTER_NUM;
	visible_cluster_srv_desc.Buffer.StructureByteStride = 0;
	visible_cluster_srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	m_device->GetDevice()->CreateShaderResourceView(in_visible_cluster_buffer.Get(), &visible_cluster_srv_desc,
		m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV][1]);


	//UAV
	D3D12_UNORDERED_ACCESS_VIEW_DESC depth_uav_desc = {};
	depth_uav_desc.Format = DXGI_FORMAT_R32_SINT;
	depth_uav_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	depth_uav_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	depth_uav_desc.Buffer.CounterOffsetInBytes = 0;
	depth_uav_desc.Buffer.FirstElement = 0;
	depth_uav_desc.Buffer.NumElements = MAX_POINTS_NUM;
	depth_uav_desc.Buffer.StructureByteStride = 0;
	m_device->GetDevice()->CreateUnorderedAccessView(out_visible_depth_buffer.Get(), nullptr, &depth_uav_desc,
		m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV][2]);
	m_device->GetDevice()->CreateUnorderedAccessView(out_visible_point_buffer.Get(), nullptr, &depth_uav_desc,
		m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV][3]);

	D3D12_UNORDERED_ACCESS_VIEW_DESC visible_points_counter_uav_desc = {};
	visible_points_counter_uav_desc.Format = DXGI_FORMAT_R32_TYPELESS;
	visible_points_counter_uav_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	visible_points_counter_uav_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
	visible_points_counter_uav_desc.Buffer.CounterOffsetInBytes = 0;
	visible_points_counter_uav_desc.Buffer.FirstElement = 0;
	visible_points_counter_uav_desc.Buffer.NumElements = 1;
	visible_points_counter_uav_desc.Buffer.StructureByteStride = 0;
	m_device->GetDevice()->CreateUnorderedAccessView(out_visible_point_counter_buffer.Get(), nullptr, &visible_points_counter_uav_desc,
		m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV][4]);


}
void MeshGaussianFillData::InitResources()
{

	return;
}
void MeshGaussianFillData::InitRootSignature()
{
	CD3DX12_ROOT_PARAMETER rootParameters[3];
	rootParameters[0].InitAsConstants(sizeof(ViewBuffer) / 4, 0, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[1].InitAsConstants(sizeof(BatchBuffer) / 4, 1, 0, D3D12_SHADER_VISIBILITY_ALL);
	CD3DX12_DESCRIPTOR_RANGE DescRange[2];
	DescRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0);//gaussian_clusters pints
	DescRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 3, 0);//visible_clusters visible_clusters_num
	rootParameters[2].InitAsDescriptorTable(_countof(DescRange), DescRange, D3D12_SHADER_VISIBILITY_ALL);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	Microsoft::WRL::ComPtr<ID3DBlob> serialized_signature_desc;
	Microsoft::WRL::ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized_signature_desc, &error));
	ThrowIfFailed(m_device->GetDevice()->CreateRootSignature(0, serialized_signature_desc->GetBufferPointer(), serialized_signature_desc->GetBufferSize(), IID_PPV_ARGS(&m_root_signature)));
}
void MeshGaussianFillData::SetRootSignature(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, const ViewInfo* p_view, const RenderProxy* proxy,
	D3D12_GPU_DESCRIPTOR_HANDLE* stack_bottom)
{
	command_list->SetComputeRootSignature(m_root_signature.Get());

	ViewBuffer view_buffer;
	view_buffer.project_transform = p_view->project_matrix;
	view_buffer.view_transform = p_view->view_matrix;
	BatchBuffer batch_buffer;
	batch_buffer.world_transform = proxy->world_transform;

	command_list->SetComputeRoot32BitConstants(0, sizeof(ViewBuffer) / 4, &view_buffer, 0);
	command_list->SetComputeRoot32BitConstants(1, sizeof(BatchBuffer) / 4, &batch_buffer, 0);
	command_list->SetComputeRootDescriptorTable(2, stack_bottom[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]);

	auto gaussian_render_proxy = dynamic_cast<const GaussianRenderProxy*>(proxy);
	assert(gaussian_render_proxy);

}
void MeshGaussianFillData::Dispatch(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
	D3DHelper::StaticDescriptorStack(&param_stacks)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
	int buffer_index,
	const ViewInfo* p_view,
	const RenderProxy* proxy)
{
	auto gaussian_render_proxy = dynamic_cast<const GaussianRenderProxy*>(proxy);
	assert(gaussian_render_proxy);
	assert(MAX_CLUSTER_NUM > proxy->GetClusterCountPerInstance());


	D3D12_GPU_DESCRIPTOR_HANDLE stack_bottom[2]{ param_stacks[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].GetTopGPU(), D3D12_GPU_DESCRIPTOR_HANDLE() };
	SetRootSignature(command_list, p_view, proxy, stack_bottom);
	gaussian_render_proxy->CommitParams(command_list, param_stacks);
	CommitDescriptors(param_stacks);
	command_list->SetPipelineState(m_pipeline_state.Get());
	command_list->Dispatch(std::ceil(proxy->GetClusterCountPerInstance() / 256.0f), 1, 1);
}



void MeshGaussianSort::InitShaders()
{
	std::vector<std::wstring> define = {
		L"-DKEY_UINT",
		L"-DSORT_PAIRS",
		L"-DPAYLOAD_UINT",
	};
	CompileComputeShader(L"./shader/GPUSorting/", L"OneSweep", L"InitSweep", define, &m_init_sweep_cs);
	CompileComputeShader(L"./shader/GPUSorting/", L"OneSweep", L"GlobalHistogram", define, &m_global_hist_cs);
	CompileComputeShader(L"./shader/GPUSorting/", L"OneSweep", L"Scan", define, &m_scan_cs);
	CompileComputeShader(L"./shader/GPUSorting/", L"OneSweep", L"DigitBinningPass", define, &m_digit_binning_pass_cs);
}

void MeshGaussianSort::InitResources()
{


	ThrowIfFailed(m_device->GetDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(k_radix * k_radixPasses * max_partition_num * sizeof(uint32_t), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
		D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_passhist_buffer)));
	m_passhist_buffer->SetName(L"pass hist");

	ThrowIfFailed(m_device->GetDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(max_ele_num * sizeof(uint32_t), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
		D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_alt_buffer)));
	m_alt_buffer->SetName(L"alt sort");

	ThrowIfFailed(m_device->GetDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(max_ele_num * sizeof(uint32_t), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
		D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_alt_payload_buffer)));
	m_alt_payload_buffer->SetName(L"alt payload");


	ThrowIfFailed(m_device->GetDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(k_radix * k_radixPasses * sizeof(uint32_t), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
		D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_globalhist_buffer)));
	m_globalhist_buffer->SetName(L"global hist");

	ThrowIfFailed(m_device->GetDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer( k_radixPasses * sizeof(uint32_t), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
		D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_index_buffer)));
	m_index_buffer->SetName(L"index");

	/*ThrowIfFailed(m_device->GetDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(uint32_t), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
		D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_err_count_buffer)));
	m_err_count_buffer->SetName(L"err count");

	ThrowIfFailed(m_device->GetDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(k_max_read_back * sizeof(uint32_t), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
		D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_readback_buffer)));
	m_readback_buffer->SetName(L"readback");*/


}

void MeshGaussianSort::InitRootSignature()
{
	CD3DX12_ROOT_PARAMETER rootParameters[8];
	rootParameters[0].InitAsConstants(4, 0);
	rootParameters[1].InitAsUnorderedAccessView((UINT)Reg::Sort);
	rootParameters[2].InitAsUnorderedAccessView((UINT)Reg::Alt);
	rootParameters[3].InitAsUnorderedAccessView((UINT)Reg::SortPayload);
	rootParameters[4].InitAsUnorderedAccessView((UINT)Reg::AltPayload);
	rootParameters[5].InitAsUnorderedAccessView((UINT)Reg::GlobalHist);
	rootParameters[6].InitAsUnorderedAccessView((UINT)Reg::PassHist);
	rootParameters[7].InitAsUnorderedAccessView((UINT)Reg::Index);
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	Microsoft::WRL::ComPtr<ID3DBlob> serialized_signature_desc;
	Microsoft::WRL::ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized_signature_desc, &error));
	ThrowIfFailed(m_device->GetDevice()->CreateRootSignature(0, serialized_signature_desc->GetBufferPointer(), serialized_signature_desc->GetBufferSize(), IID_PPV_ARGS(&m_root_signature)));

}
void MeshGaussianSort::SetRootSignature(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, const ViewInfo* p_view, const RenderProxy* proxy,
	D3D12_GPU_DESCRIPTOR_HANDLE stack_bottom[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES])
{
	assert(false);
}
void MeshGaussianSort::SetRootSignature(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, uint32_t element_num,
	Microsoft::WRL::ComPtr<ID3D12Resource> in_out_sort_buffer, Microsoft::WRL::ComPtr<ID3D12Resource> in_out_payload_buffer)
{

	command_list->SetComputeRootSignature(m_root_signature.Get());
	command_list->SetComputeRootUnorderedAccessView(1 + (UINT)Reg::Sort, in_out_sort_buffer->GetGPUVirtualAddress());
	command_list->SetComputeRootUnorderedAccessView(1 + (UINT)Reg::Alt, m_alt_buffer->GetGPUVirtualAddress());
	command_list->SetComputeRootUnorderedAccessView(1 + (UINT)Reg::SortPayload, in_out_payload_buffer->GetGPUVirtualAddress());
	command_list->SetComputeRootUnorderedAccessView(1 + (UINT)Reg::AltPayload, m_alt_payload_buffer->GetGPUVirtualAddress());
	command_list->SetComputeRootUnorderedAccessView(1 + (UINT)Reg::GlobalHist, m_globalhist_buffer->GetGPUVirtualAddress());
	command_list->SetComputeRootUnorderedAccessView(1 + (UINT)Reg::PassHist, m_passhist_buffer->GetGPUVirtualAddress());
	command_list->SetComputeRootUnorderedAccessView(1 + (UINT)Reg::Index, m_index_buffer->GetGPUVirtualAddress());
}

void MeshGaussianSort::InitPSO()
{
	D3D12_COMPUTE_PIPELINE_STATE_DESC pso_desc = {};
	pso_desc.CS = CD3DX12_SHADER_BYTECODE(m_init_sweep_cs.Get());
	pso_desc.pRootSignature = m_root_signature.Get();
	auto psoStream = CD3DX12_PIPELINE_STATE_STREAM(pso_desc);
	D3D12_PIPELINE_STATE_STREAM_DESC streamDesc;
	streamDesc.pPipelineStateSubobjectStream = &psoStream;
	streamDesc.SizeInBytes = sizeof(psoStream);
	ThrowIfFailed(m_device->GetDevice()->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&m_init_sweep_pso)));

	pso_desc.CS = CD3DX12_SHADER_BYTECODE(m_global_hist_cs.Get());
	psoStream = CD3DX12_PIPELINE_STATE_STREAM(pso_desc);
	streamDesc.pPipelineStateSubobjectStream = &psoStream;
	streamDesc.SizeInBytes = sizeof(psoStream);
	ThrowIfFailed(m_device->GetDevice()->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&m_global_hist_pso)));

	pso_desc.CS = CD3DX12_SHADER_BYTECODE(m_scan_cs.Get());
	psoStream = CD3DX12_PIPELINE_STATE_STREAM(pso_desc);
	streamDesc.pPipelineStateSubobjectStream = &psoStream;
	streamDesc.SizeInBytes = sizeof(psoStream);
	ThrowIfFailed(m_device->GetDevice()->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&m_scan_pso)));

	pso_desc.CS = CD3DX12_SHADER_BYTECODE(m_digit_binning_pass_cs.Get());
	psoStream = CD3DX12_PIPELINE_STATE_STREAM(pso_desc);
	streamDesc.pPipelineStateSubobjectStream = &psoStream;
	streamDesc.SizeInBytes = sizeof(psoStream);
	ThrowIfFailed(m_device->GetDevice()->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&m_digit_binning_pass_pso)));

}

MeshGaussianSort::MeshGaussianSort():ComputePipeline()
{

}

void MeshGaussianSort::Dispatch(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
	D3DHelper::StaticDescriptorStack(&param_stacks)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
	int buffer_index,
	const ViewInfo* p_view,
	const RenderProxy* proxy)
{
	assert(false);
}
void MeshGaussianSort::Dispatch(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, uint32_t ele_num,
	Microsoft::WRL::ComPtr<ID3D12Resource> in_out_sort_buffer, Microsoft::WRL::ComPtr<ID3D12Resource> in_out_payload_buffer)
{
	assert(ele_num < max_ele_num);
	SetRootSignature(command_list, ele_num, in_out_sort_buffer, in_out_payload_buffer);


	const uint32_t partitions = std::ceil(ele_num / (float)partitionSize);
	{
		//init sweep
		command_list->SetPipelineState(m_init_sweep_pso.Get());

		uint32_t threadBlocks = partitions;
		std::vector<uint32_t> t = { 0, 0, threadBlocks, 0 };
		command_list->SetComputeRoot32BitConstants(0, (uint32_t)t.size(), t.data(), 0);
		command_list->Dispatch(256, 1, 1);

		D3D12_RESOURCE_BARRIER uav_barriers[] = {
			CD3DX12_RESOURCE_BARRIER::UAV(m_globalhist_buffer.Get()),
			CD3DX12_RESOURCE_BARRIER::UAV(m_passhist_buffer.Get()),
			CD3DX12_RESOURCE_BARRIER::UAV(m_index_buffer.Get()),
		};
		command_list->ResourceBarrier(_countof(uav_barriers), uav_barriers);
	}

	
	uint32_t globalHistPartitions = std::ceil(ele_num / (float)k_globalHistPartitionSize);
	{
		//global hist
		command_list->SetPipelineState(m_global_hist_pso.Get());

		uint32_t threadBlocks = globalHistPartitions;
		uint32_t fullBlocks = partitions / k_maxDim;
		if (fullBlocks)
		{
			std::vector<uint32_t> t = { ele_num,0,threadBlocks,k_isNotPartialBitFlag };
			command_list->SetComputeRoot32BitConstants(0, (uint32_t)t.size(), t.data(), 0);
			command_list->Dispatch(k_maxDim, fullBlocks, 1);
		}
		uint32_t partialBlocks = threadBlocks - fullBlocks * k_maxDim;
		if (partialBlocks)
		{
			std::vector<uint32_t> t = { ele_num,0,threadBlocks,fullBlocks << 1 | k_isPartialBitFlag };
			command_list->SetComputeRoot32BitConstants(0, (uint32_t)t.size(), t.data(), 0);
			command_list->Dispatch(partialBlocks, 1, 1);
		}
		D3D12_RESOURCE_BARRIER uav_barriers[] = {
			CD3DX12_RESOURCE_BARRIER::UAV(m_globalhist_buffer.Get())
		};
		command_list->ResourceBarrier(_countof(uav_barriers), uav_barriers);
	}
	
	{
		//scan
		command_list->SetPipelineState(m_scan_pso.Get());

		uint32_t threadBlocks = partitions;
		std::vector<uint32_t> t = { 0, 0, threadBlocks, 0 };
		command_list->SetComputeRoot32BitConstants(0, (uint32_t)t.size(), t.data(), 0);
		command_list->Dispatch(k_radixPasses, 1, 1);

		D3D12_RESOURCE_BARRIER uav_barriers[] = {
			CD3DX12_RESOURCE_BARRIER::UAV(m_passhist_buffer.Get())
		};
		command_list->ResourceBarrier(_countof(uav_barriers), uav_barriers);
	}

	
	{
		//pass

		command_list->SetPipelineState(m_digit_binning_pass_pso.Get());
		uint32_t threadBlocks = partitions;
		const uint32_t fullBlocks = threadBlocks / k_maxDim;
		const uint32_t partialBlocks = threadBlocks - fullBlocks * k_maxDim;
		Microsoft::WRL::ComPtr<ID3D12Resource> alt_buffer = m_alt_buffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> alt_payload_buffer = m_alt_payload_buffer;

		for (uint32_t radixShift = 0; radixShift < 32; radixShift += 8)
		{
			command_list->SetComputeRootUnorderedAccessView(1 + (UINT)Reg::Sort, in_out_sort_buffer->GetGPUVirtualAddress());
			command_list->SetComputeRootUnorderedAccessView(1 + (UINT)Reg::Alt, alt_buffer->GetGPUVirtualAddress());
			command_list->SetComputeRootUnorderedAccessView(1 + (UINT)Reg::SortPayload, in_out_payload_buffer->GetGPUVirtualAddress());
			command_list->SetComputeRootUnorderedAccessView(1 + (UINT)Reg::AltPayload, alt_payload_buffer->GetGPUVirtualAddress());

			if (fullBlocks)
			{
				std::vector<uint32_t> t = { ele_num, radixShift, threadBlocks, 0 };
				command_list->SetComputeRoot32BitConstants(0, (uint32_t)t.size(), t.data(), 0);
				command_list->Dispatch(k_maxDim, fullBlocks, 1);
				auto pass_hist_uav_barrier = CD3DX12_RESOURCE_BARRIER::UAV(m_passhist_buffer.Get());
				//To be absolutely safe, add a barrier here on the pass histogram
				//As threadblocks in the second dispatch are dependent on the first dispatch
				command_list->ResourceBarrier(1, &pass_hist_uav_barrier);
			}
			if (partialBlocks)
			{
				std::vector<uint32_t> t = { ele_num, radixShift, threadBlocks, 0 };
				command_list->SetComputeRoot32BitConstants(0, (uint32_t)t.size(), t.data(), 0);
				command_list->Dispatch(partialBlocks, 1, 1);
			}

			D3D12_RESOURCE_BARRIER uav_barriers[] = {
				CD3DX12_RESOURCE_BARRIER::UAV(in_out_sort_buffer.Get()),
				CD3DX12_RESOURCE_BARRIER::UAV(in_out_payload_buffer.Get()),
				CD3DX12_RESOURCE_BARRIER::UAV(alt_payload_buffer.Get()),
				CD3DX12_RESOURCE_BARRIER::UAV(alt_buffer.Get()),
			};
			command_list->ResourceBarrier(_countof(uav_barriers), uav_barriers);
			Microsoft::WRL::ComPtr<ID3D12Resource> temp;
			temp = in_out_sort_buffer;
			in_out_sort_buffer = alt_buffer;
			alt_buffer = temp;
			temp = in_out_payload_buffer;
			in_out_payload_buffer = alt_payload_buffer;
			alt_payload_buffer = temp;
		}
	}
	
	
}




MeshGaussianRaster::MeshGaussianRaster() :DefaultGraphicPipeline(), pipeline_data_dirty(true)
{

}

void MeshGaussianRaster::Init(std::shared_ptr<D3DHelper::Device> device, Microsoft::WRL::ComPtr<ID3D12Resource> in_counter_buffer,
	Microsoft::WRL::ComPtr<ID3D12Resource> in_visible_cluster_buffer,const int MAX_CLUSTER_NUM)
{
	DefaultGraphicPipeline::Init(device);

	//init descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};
	heap_desc.NumDescriptors = 3;
	heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].Init(heap_desc, m_device);

	//init srv: gaussian texture
	D3D12_SHADER_RESOURCE_VIEW_DESC gaussian_texture_srv_desc;
	gaussian_texture_srv_desc.Format = DXGI_FORMAT_R32_FLOAT;
	gaussian_texture_srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	gaussian_texture_srv_desc.Texture2D.MipLevels = GAUSSIAN_TEXTURE_LOD;
	gaussian_texture_srv_desc.Texture2D.PlaneSlice = 0;
	gaussian_texture_srv_desc.Texture2D.MostDetailedMip = 0;
	gaussian_texture_srv_desc.Texture2D.ResourceMinLODClamp = 0;
	gaussian_texture_srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	m_device->GetDevice()->CreateShaderResourceView(gaussian_texture_buffer.Get(), &gaussian_texture_srv_desc,
		m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV][0]);

	//init srv: counter
	D3D12_SHADER_RESOURCE_VIEW_DESC counter_srv_desc;
	counter_srv_desc.Format = DXGI_FORMAT_R32_TYPELESS;
	counter_srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	counter_srv_desc.Buffer.FirstElement = 0;
	counter_srv_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
	counter_srv_desc.Buffer.NumElements = 1;
	counter_srv_desc.Buffer.StructureByteStride = 0;
	counter_srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	m_device->GetDevice()->CreateShaderResourceView(in_counter_buffer.Get(), &counter_srv_desc,
		m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV][1]);

	//init srv: visible_cluster
	D3D12_SHADER_RESOURCE_VIEW_DESC visible_cluster_srv_desc;
	visible_cluster_srv_desc.Format = DXGI_FORMAT_R32_SINT;
	visible_cluster_srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	visible_cluster_srv_desc.Buffer.FirstElement = 0;
	visible_cluster_srv_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	visible_cluster_srv_desc.Buffer.NumElements = MAX_CLUSTER_NUM;
	visible_cluster_srv_desc.Buffer.StructureByteStride = 0;
	visible_cluster_srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	m_device->GetDevice()->CreateShaderResourceView(in_visible_cluster_buffer.Get(), &visible_cluster_srv_desc,
		m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV][2]);

	

}

void MeshGaussianRaster::InitShaders()
{
//	{
//		LPCWSTR hlsl_file_name = L"./shader/ms_gaussian_splatting/GaussianAS.hlsl";
//		LPCWSTR pszArgs[] =
//		{
//			hlsl_file_name,            // shader source file name for error reporting and for PIX shader source view.  
//			L"-E", L"main",              // Entry point.
//			L"-T", L"as_6_5",            // Target.
//			L"-Zi",L"-Qembed_debug",                      // Enable debug information (slim format)
//#if defined(_DEBUG)
//			L"-Od",
//#endif
//			//L"-D", L"MYDEFINE=1",        // A single define.
//			L"-Fd", L"./shader/ms_gaussian_splatting/GaussianPS.pdb",     // The file name of the pdb. This must either be supplied or the autogenerated file name must be used.
//			L"-Fo",L"./shader/ms_gaussian_splatting/GaussianPS.cso",
//			L"-Qstrip_reflect",          // Strip reflection into a separate blob. 
//		};
//		Microsoft::WRL::ComPtr<IDxcBlob> shader_out;
//		CompileShaingModel6(pszArgs, _countof(pszArgs), hlsl_file_name, shader_out);
//		shader_out.As(&m_amp_shader);
//	}

	{
		LPCWSTR hlsl_file_name = L"./shader/ms_gaussian_splatting/GaussianMS.hlsl";
		LPCWSTR pszArgs[] =
		{
			hlsl_file_name,            // shader source file name for error reporting and for PIX shader source view.  
			L"-E", L"main",              // Entry point.
			L"-T", L"ms_6_5",            // Target.
			L"-Zi", L"-Qembed_debug",                     // Enable debug information (slim format)
#if defined(_DEBUG)
			L"-Od",
#endif
			//L"-D", L"MYDEFINE=1",        // A single define.
			L"-Fd", L"./shader/ms_gaussian_splatting/GaussianMS.pdb",     // The file name of the pdb. This must either be supplied or the autogenerated file name must be used.
			L"-Fo",L"./shader/ms_gaussian_splatting/GaussianMS.cso"
		};
		Microsoft::WRL::ComPtr<IDxcBlob> shader_out;
		CompileShaingModel6(pszArgs, _countof(pszArgs), hlsl_file_name, shader_out);
		shader_out.As(&m_vertex_shader);
	}

	{
		LPCWSTR hlsl_file_name = L"./shader/ms_gaussian_splatting/GaussianPS.hlsl";
		LPCWSTR pszArgs[] =
		{
			hlsl_file_name,            // shader source file name for error reporting and for PIX shader source view.  
			L"-E", L"PSMain",              // Entry point.
			L"-T", L"ps_6_0",            // Target.
			L"-Zi",L"-Qembed_debug",                      // Enable debug information (slim format)
#if defined(_DEBUG)
			L"-Od",
#endif
			//L"-D", L"MYDEFINE=1",        // A single define.
			L"-Fd", L"./shader/ms_gaussian_splatting/GaussianPS.pdb",     // The file name of the pdb. This must either be supplied or the autogenerated file name must be used.
			L"-Fo",L"./shader/ms_gaussian_splatting/GaussianPS.cso",
			L"-Qstrip_reflect",          // Strip reflection into a separate blob. 
		};
		Microsoft::WRL::ComPtr<IDxcBlob> shader_out;
		CompileShaingModel6(pszArgs, _countof(pszArgs), hlsl_file_name, shader_out);
		shader_out.As(&m_pixel_shader);
	}
}

void MeshGaussianRaster::InitPSO()
{
	D3D12_BLEND_DESC blend_desc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	blend_desc.RenderTarget[0].BlendEnable = true;
	blend_desc.RenderTarget[0].LogicOpEnable = FALSE;
	blend_desc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blend_desc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blend_desc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blend_desc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blend_desc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blend_desc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;

	//depth:disable Depth Wirte & enable Depth Test
	D3D12_DEPTH_STENCIL_DESC depth_stencil_desc;
	depth_stencil_desc.DepthEnable = true;
	depth_stencil_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depth_stencil_desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depth_stencil_desc.StencilEnable = false;

	D3DX12_MESH_SHADER_PIPELINE_STATE_DESC pso_desc = {};
	pso_desc.pRootSignature = m_root_signature.Get();
	//pso_desc.AS =  CD3DX12_SHADER_BYTECODE(m_amp_shader.Get());
	pso_desc.MS = CD3DX12_SHADER_BYTECODE(m_vertex_shader.Get());
	pso_desc.PS = CD3DX12_SHADER_BYTECODE(m_pixel_shader.Get());
	pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pso_desc.NumRenderTargets = 1;
	pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	pso_desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	pso_desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; // do not cull back!!
	pso_desc.BlendState = blend_desc;         // transparent
	pso_desc.DepthStencilState = depth_stencil_desc;
	pso_desc.SampleMask = UINT_MAX;
	pso_desc.SampleDesc = DefaultSampleDesc();

	auto psoStream = CD3DX12_PIPELINE_MESH_STATE_STREAM(pso_desc);
	D3D12_PIPELINE_STATE_STREAM_DESC streamDesc;
	streamDesc.pPipelineStateSubobjectStream = &psoStream;
	streamDesc.SizeInBytes = sizeof(psoStream);
	ThrowIfFailed(m_device->GetDevice()->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&m_pipeline_state)));
}

void MeshGaussianRaster::InitResources()
{
	auto gaussian_texture_desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32_FLOAT, GAUSSIAN_TEXTURE_SIZE.x, GAUSSIAN_TEXTURE_SIZE.y, 1, GAUSSIAN_TEXTURE_LOD, 1, 0);
	ThrowIfFailed(m_device->GetDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &gaussian_texture_desc,
		D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&gaussian_texture_buffer)));

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(gaussian_texture_buffer.Get(), 0, GAUSSIAN_TEXTURE_LOD);
	ThrowIfFailed(m_device->GetDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&gaussian_texture_upload_buffer)));

	//init cpu data
	cpu_gaussian_texture_buffer.resize(GAUSSIAN_TEXTURE_LOD);
	cpu_gaussian_texture_buffer[0].resize(GAUSSIAN_TEXTURE_SIZE.x * GAUSSIAN_TEXTURE_SIZE.y);
	{
		auto& cpu_texture = cpu_gaussian_texture_buffer[0];
		float mean_x = GAUSSIAN_TEXTURE_SIZE.x / 2;
		float mean_y = GAUSSIAN_TEXTURE_SIZE.y / 2;
		float axis_x = mean_x - 0.5f;
		float axis_y = mean_y - 0.5f;
		float cov_inv_00 = 2 * std::log(255) / (axis_x * axis_x);
		float cov_inv_11 = 2 * std::log(255) / (axis_y * axis_y);
		for (int x = 0; x < GAUSSIAN_TEXTURE_SIZE.x; x++)
		{
			for (int y = 0; y < GAUSSIAN_TEXTURE_SIZE.y; y++)
			{
				float delta_x = x + 0.5f - mean_x;
				float delta_y = y + 0.5f - mean_y;
				cpu_texture[y * GAUSSIAN_TEXTURE_SIZE.x + x] = std::exp(-0.5 * (cov_inv_00 * delta_x * delta_x + cov_inv_11 * delta_y * delta_y));
			}
		}
	}
	for (int lod = 1; lod < GAUSSIAN_TEXTURE_LOD; lod++)
	{
		int cur_lod_size_x = (GAUSSIAN_TEXTURE_SIZE.x >> lod);
		int cur_lod_size_y = (GAUSSIAN_TEXTURE_SIZE.y >> lod);
		cpu_gaussian_texture_buffer[lod].resize(cur_lod_size_x * cur_lod_size_y);
		for (int x = 0; x < cur_lod_size_x; x++)
		{
			for (int y = 0; y < cur_lod_size_y; y++)
			{
				float value = cpu_gaussian_texture_buffer[lod - 1][(y * 2) * (cur_lod_size_x * 2) + (x * 2)];
				value += cpu_gaussian_texture_buffer[lod - 1][(y * 2 + 1) * (cur_lod_size_x * 2) + (x * 2)];
				value += cpu_gaussian_texture_buffer[lod - 1][(y * 2) * (cur_lod_size_x * 2) + (x * 2 + 1)];
				value += cpu_gaussian_texture_buffer[lod - 1][(y * 2 + 1) * (cur_lod_size_x * 2) + (x * 2 + 1)];
				cpu_gaussian_texture_buffer[lod][y * cur_lod_size_x + x] = value / 4.0f;
			}
		}
	}
	
	return;
}

void MeshGaussianRaster::UpdatePipelineData(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list)
{
	assert(pipeline_data_dirty);
	std::vector<D3D12_SUBRESOURCE_DATA> texture_lod_data;
	texture_lod_data.resize(GAUSSIAN_TEXTURE_LOD);
	for (int i = 0; i < GAUSSIAN_TEXTURE_LOD; i++)
	{
		texture_lod_data[i].pData = cpu_gaussian_texture_buffer[i].data();
		texture_lod_data[i].RowPitch = (GAUSSIAN_TEXTURE_SIZE.x >> i) * sizeof(float);
		texture_lod_data[i].SlicePitch = cpu_gaussian_texture_buffer[i].size() * sizeof(float);
	}

	UpdateSubresources(command_list.Get(), gaussian_texture_buffer.Get(), gaussian_texture_upload_buffer.Get(), 0, 0, GAUSSIAN_TEXTURE_LOD, texture_lod_data.data());
	command_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(gaussian_texture_buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	pipeline_data_dirty = false;
}

void MeshGaussianRaster::InitRootSignature()
{
	D3D12_STATIC_SAMPLER_DESC sampler = CD3DX12_STATIC_SAMPLER_DESC();
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = GAUSSIAN_TEXTURE_LOD;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	CD3DX12_ROOT_PARAMETER rootParameters[3];
	rootParameters[0].InitAsConstants(sizeof(ViewBuffer) / 4, 0, 0, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[1].InitAsConstants(sizeof(BatchBuffer) / 4, 1, 0, D3D12_SHADER_VISIBILITY_ALL);
	CD3DX12_DESCRIPTOR_RANGE DescRange[1];
	DescRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0);//points srv + clusters srv + texture srv + counter + visible_buffer
	rootParameters[2].InitAsDescriptorTable(_countof(DescRange), DescRange, D3D12_SHADER_VISIBILITY_ALL);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	Microsoft::WRL::ComPtr<ID3DBlob> serialized_signature_desc;
	Microsoft::WRL::ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized_signature_desc, &error));
	ThrowIfFailed(m_device->GetDevice()->CreateRootSignature(0, serialized_signature_desc->GetBufferPointer(), serialized_signature_desc->GetBufferSize(), IID_PPV_ARGS(&m_root_signature)));
}

void MeshGaussianRaster::SetRootSignature(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, const ViewInfo* p_view, const RenderProxy* proxy,
	D3D12_GPU_DESCRIPTOR_HANDLE* stack_bottom)
{
	command_list->SetGraphicsRootSignature(m_root_signature.Get());

	ViewBuffer view_buffer;
	view_buffer.view_transform = p_view->view_matrix;
	view_buffer.project_transform = p_view->project_matrix;
	view_buffer.viewport_size = { int(p_view->m_viewport.Width),int(p_view->m_viewport.Height) };
	view_buffer.focal = p_view->focal;
	BatchBuffer batch_buffer;
	batch_buffer.world_transform = proxy->world_transform;
	batch_buffer.clusters_num = proxy->GetClusterCountPerInstance();

	command_list->SetGraphicsRoot32BitConstants(0, sizeof(ViewBuffer) / 4, &view_buffer, 0);
	command_list->SetGraphicsRoot32BitConstants(1, sizeof(BatchBuffer) / 4, &batch_buffer, 0);
	command_list->SetGraphicsRootDescriptorTable(2, stack_bottom[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]);

	auto gaussian_render_proxy = dynamic_cast<const GaussianRenderProxy*>(proxy);
	assert(gaussian_render_proxy);

}

void MeshGaussianRaster::CommitDescriptors(D3DHelper::StaticDescriptorStack(&param_stacks)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES])
{
	param_stacks[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].push_back(m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV],
		0, m_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].GetDescriptorNum());
}

void MeshGaussianRaster::Draw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
	D3DHelper::StaticDescriptorStack(&param_stacks)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
	int buffer_index,
	const ViewInfo* p_view,
	const RenderProxy* proxy)
{
	auto gaussian_render_proxy = dynamic_cast<const GaussianRenderProxy*>(proxy);
	assert(gaussian_render_proxy);
	if (pipeline_data_dirty)
	{
		UpdatePipelineData(command_list);
	}

	command_list->RSSetViewports(1, &p_view->m_viewport);
	command_list->RSSetScissorRects(1, &p_view->m_scissor_rect);
	command_list->OMSetRenderTargets(1, &p_view->render_target_view, false, &p_view->depth_stencil_view);
	command_list->SetPipelineState(m_pipeline_state.Get());
	D3D12_GPU_DESCRIPTOR_HANDLE stack_bottom[2]{ param_stacks[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].GetTopGPU(), D3D12_GPU_DESCRIPTOR_HANDLE() };
	SetRootSignature(command_list, p_view, proxy, stack_bottom);
	gaussian_render_proxy->CommitParams(command_list, param_stacks);
	CommitDescriptors(param_stacks);

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList6> command_list6;
	command_list.As(&command_list6);
	assert(command_list6);
	command_list6->DispatchMesh(proxy->GetClusterCountPerInstance(), 1, 1);
}