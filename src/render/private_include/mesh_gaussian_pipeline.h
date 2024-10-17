#include"pipeline_object.h"

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

	Microsoft::WRL::ComPtr<ID3DBlob> m_clear_shader;
	Microsoft::WRL::ComPtr<ID3DBlob> m_culling_shader;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_clear_pso;

	Microsoft::WRL::ComPtr<ID3D12Resource> counter_buffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> visible_cluster_buffer;
	const int MAX_CLUSTER_NUM = 1024 * 1024;

	virtual void InitShaders();
	virtual void InitRootSignature();
	virtual void InitPSO();
	virtual void InitResources();

	virtual void SetRootSignature(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, const ViewInfo* p_view, const RenderProxy* proxy,
		D3D12_GPU_DESCRIPTOR_HANDLE* stack_bottom);
	virtual void CommitDescriptors(D3DHelper::StaticDescriptorStack(&param_stacks)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES]);

public:
	MeshGaussianClusterCulling();
	virtual void Draw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
		D3DHelper::StaticDescriptorStack(&param_stacks)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
		int buffer_index,
		const ViewInfo* p_view,
		const RenderProxy* proxy) {};
	virtual void Dispatch(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
		D3DHelper::StaticDescriptorStack(&param_stacks)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES],
		int buffer_index,
		const ViewInfo* p_view,
		const RenderProxy* proxy);
};

class MeshGaussianPipeline :public DefaultGraphicPipeline
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
		int clusters_num;
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
