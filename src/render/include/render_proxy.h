#pragma
#include<memory>
#include "render_dll_helper.h"
#include"DirectXMath.h"
#include "d3d_resources.h"
#include "view_info.h"
class ShadingModel;
class RENDER_MODULE_API RenderProxy
{
	
public:
	std::shared_ptr<ShadingModel> shading_model;
	DirectX::XMMATRIX world_transform;
	std::shared_ptr<unsigned char> custom_data;
	bool b_render_resources_inited;
	virtual void IASet(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list) const = 0;
	virtual void RSSet(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, const ViewInfo& view) const = 0;
	virtual void OMSet(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, const ViewInfo& view) const = 0;
	virtual void Draw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list) const = 0;
	virtual void PopulateCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,const ViewInfo& view,int buffer_index)=0;
	virtual void InitRenderResources()=0;
	virtual void UploadStatic()=0;
	virtual void UploadDynamic(int game_frame) = 0;
};


class RENDER_MODULE_API TriangleRenderProxy:public RenderProxy
{
public:
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};
	struct DeviceStaticResource
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> m_vertex_buffer;
		D3D12_VERTEX_BUFFER_VIEW m_vertex_buffer_view;
	};
public:
	std::vector<Vertex> vertex;
	std::unique_ptr<DeviceStaticResource> device_static_resource;
	virtual void IASet(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list) const;
	virtual void RSSet(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, const ViewInfo& view) const;
	virtual void OMSet(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, const ViewInfo& view) const;
	virtual void Draw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list) const;
	virtual void PopulateCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,const ViewInfo& view, int buffer_index);
	virtual void InitRenderResources();
	virtual void UploadStatic();
	virtual void UploadDynamic(int game_frame);
};


class RENDER_MODULE_API GaussianRenderProxy :public RenderProxy
{
public:
	struct GaussianPoint
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT3X3 cov3d;
	};
	struct GaussianCluster
	{
		int points_num;
		int point_offset;
		DirectX::XMFLOAT3 origin;
		DirectX::XMFLOAT3 extension;
	};

	struct DeviceStaticResource
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> m_points_buffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_clusters_buffer;
		const int points_srv_index = 0;
		const int clusters_srv_index = 1;
		D3dDescriptorHeapHelper descriptor_heap;
	};
public:
	std::vector<GaussianPoint> points_buffer;
	std::vector<GaussianCluster> clusters_buffer;
	std::unique_ptr<DeviceStaticResource> device_static_resource;
	std::unique_ptr<DeviceStaticResource> device_upload_resource;
	bool bUploadHeapUpdated;
	virtual void IASet(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list) const {};
	virtual void RSSet(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, const ViewInfo& view) const {};
	virtual void OMSet(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, const ViewInfo& view) const {};
	virtual void Draw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list) const {};
	virtual void PopulateCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, const ViewInfo& view, int buffer_index);
	virtual void InitRenderResources();
	virtual void UploadStatic();
	virtual void UploadDynamic(int game_frame) {};
	virtual void UpdateDefaultHeap(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list);
};