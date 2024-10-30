#pragma once
#include"pipeline_object.h"

class MeshGaussianClear :public ComputePipeline
{
protected:

	virtual void InitRootSignature();
	virtual void InitResources();

	virtual void SetRootSignature(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, const ViewInfo* p_view, const RenderProxy* proxy,
		D3D12_GPU_DESCRIPTOR_HANDLE* stack_bottom);
public:
	MeshGaussianClear();
	virtual void Init(std::shared_ptr<D3DHelper::Device> device,
		Microsoft::WRL::ComPtr<ID3D12Resource> out_visible_cluster_counter_buffer,
		Microsoft::WRL::ComPtr<ID3D12Resource> out_visible_point_counter_buffer,
		Microsoft::WRL::ComPtr<ID3D12Resource> out_filldata_arg_buffer,
		Microsoft::WRL::ComPtr<ID3D12Resource> out_filldata_sort_buffer);
	virtual void Dispatch(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
		D3DHelper::StaticDescriptorStack(&param_stacks)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
		int buffer_index,
		const ViewInfo* p_view,
		const RenderProxy* proxy);
};

class MeshGaussianClusterCulling:public ComputePipeline
{
protected:
	struct ViewBuffer
	{
		DirectX::XMFLOAT4 plane[6];
	};
	struct BatchBuffer
	{
		int clusters_num;
	};
	int MAX_CLUSTER_NUM;

	virtual void InitRootSignature();
	virtual void InitResources();
	virtual void SetRootSignature(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, const ViewInfo* p_view, const RenderProxy* proxy,
		D3D12_GPU_DESCRIPTOR_HANDLE* stack_bottom);
public:
	MeshGaussianClusterCulling();
	virtual void Init(std::shared_ptr<D3DHelper::Device> device, Microsoft::WRL::ComPtr<ID3D12Resource> out_counter_buffer,
		Microsoft::WRL::ComPtr<ID3D12Resource> out_visible_cluster_buffer, Microsoft::WRL::ComPtr<ID3D12Resource> out_indirect_arg,
		const int MAX_CLUSTER_NUM);
	virtual void Dispatch(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
		D3DHelper::StaticDescriptorStack(&param_stacks)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
		int buffer_index,
		const ViewInfo* p_view,
		const RenderProxy* proxy);
};

class MeshGaussianFillData :public ComputePipeline
{
protected:
	struct ViewBuffer
	{
		DirectX::XMMATRIX view_transform;
		DirectX::XMMATRIX project_transform;
	};
	struct BatchBuffer
	{
		DirectX::XMMATRIX world_transform;
	};
	int MAX_CLUSTER_NUM;
	int MAX_POINTS_NUM;

	virtual void InitRootSignature();
	virtual void InitResources();

	virtual void SetRootSignature(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, const ViewInfo* p_view, const RenderProxy* proxy,
		D3D12_GPU_DESCRIPTOR_HANDLE* stack_bottom);
public:
	MeshGaussianFillData();
	virtual void Init(std::shared_ptr<D3DHelper::Device> device,
		Microsoft::WRL::ComPtr<ID3D12Resource> in_visible_cluster_counter_buffer,
		Microsoft::WRL::ComPtr<ID3D12Resource> in_visible_cluster_buffer,
		Microsoft::WRL::ComPtr<ID3D12Resource> out_visible_point_buffer,
		Microsoft::WRL::ComPtr<ID3D12Resource> out_visible_depth_buffer,
		Microsoft::WRL::ComPtr<ID3D12Resource> out_visible_point_counter_buffer,
		Microsoft::WRL::ComPtr<ID3D12Resource> out_indirect_arg_sort_buffer,
		const int MAX_CLUSTER_NUM,const int MAX_POINTS_NUM);
	virtual void Dispatch(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
		D3DHelper::StaticDescriptorStack(&param_stacks)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
		int buffer_index,
		const ViewInfo* p_view,
		const RenderProxy* proxy);
	virtual void DispatchIndirect(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
		Microsoft::WRL::ComPtr<ID3D12Resource> indirect_args,
		D3DHelper::StaticDescriptorStack(&param_stacks)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
		int buffer_index,
		const ViewInfo* p_view,
		const RenderProxy* proxy);
};

class MeshGaussianSort :public ComputePipeline
{
protected:
	enum class Reg
	{
		Sort = 0,
		Alt = 1,
		SortPayload = 2,
		AltPayload = 3,
		GlobalHist = 4,
		PassHist = 5,
		Index = 6,
	};

	const uint32_t lanesPerWarp = 32;
	const uint32_t k_radix = 256;
	const uint32_t k_radixPasses = 4;
	const uint32_t k_max_read_back = 8192;
	const uint32_t keysPerThread = 15;
	const uint32_t threadsPerThreadBlock = 512;
	const uint32_t k_maxDim = 65535;
	const uint32_t k_isNotPartialBitFlag = 0;
	const uint32_t k_isPartialBitFlag = 1;
	const uint32_t k_globalHistPartitionSize = 32768;

	const uint32_t partitionSize= keysPerThread * threadsPerThreadBlock;
	const uint32_t max_partition_num= 1024;
	const uint32_t max_ele_num= max_partition_num * partitionSize;
	

	Microsoft::WRL::ComPtr<ID3DBlob> m_init_sweep_cs;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_init_sweep_pso;
	Microsoft::WRL::ComPtr<ID3DBlob> m_global_hist_cs;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_global_hist_pso;
	Microsoft::WRL::ComPtr<ID3DBlob> m_scan_cs;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_scan_pso;
	Microsoft::WRL::ComPtr<ID3DBlob> m_digit_binning_pass_cs;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_digit_binning_pass_pso;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_passhist_buffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_alt_buffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_alt_payload_buffer;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_globalhist_buffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_index_buffer;
	//Microsoft::WRL::ComPtr<ID3D12Resource> m_err_count_buffer;
	//Microsoft::WRL::ComPtr<ID3D12Resource> m_readback_buffer;

	virtual void InitShaders();
	virtual void InitPSO();
	virtual void InitResources();
	virtual void InitRootSignature();
	virtual void SetRootSignature(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, 
		Microsoft::WRL::ComPtr<ID3D12Resource> element_num, Microsoft::WRL::ComPtr<ID3D12Resource> blocks_num,
		Microsoft::WRL::ComPtr<ID3D12Resource> in_out_sort_buffer, Microsoft::WRL::ComPtr<ID3D12Resource> in_out_payload_buffer);
	virtual void SetRootSignature(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, const ViewInfo* p_view, const RenderProxy* proxy,
		D3D12_GPU_DESCRIPTOR_HANDLE stack_bottom[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES]);
public:
	MeshGaussianSort();
	virtual void Dispatch(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, uint32_t ele_num, 
		Microsoft::WRL::ComPtr<ID3D12Resource> ele_num_buffer, Microsoft::WRL::ComPtr<ID3D12Resource> block_num_buffer,
		Microsoft::WRL::ComPtr<ID3D12Resource> in_out_sort_buffer, Microsoft::WRL::ComPtr<ID3D12Resource> in_out_payload_buffer);
	virtual void Dispatch(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
		D3DHelper::StaticDescriptorStack(&param_stacks)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
		int buffer_index,
		const ViewInfo* p_view,
		const RenderProxy* proxy);
};


class MeshGaussianRaster :public DefaultGraphicPipeline
{
protected:
	Microsoft::WRL::ComPtr<ID3DBlob> m_amp_shader;

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

	MeshGaussianRaster();
	virtual void Init(std::shared_ptr<D3DHelper::Device> device, Microsoft::WRL::ComPtr<ID3D12Resource> in_counter_buffer,
		Microsoft::WRL::ComPtr<ID3D12Resource> in_visible_cluster_buffer, const int MAX_POINTS_NUM);
	//todo indirect dispatch
	virtual void Draw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
		D3DHelper::StaticDescriptorStack(&param_stacks)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
		int buffer_index,
		const ViewInfo* p_view,
		const RenderProxy* proxy);
};
