#pragma once
#include<memory>
#include "render_dll_helper.h"
#include"DirectXMath.h"
#include "view_info.h"

class ShadingModel;
namespace D3DHelper {
	class Device;
	class StaticDescriptorStack;
};

class RENDER_MODULE_API RenderProxy
{
	
public:
	std::shared_ptr<ShadingModel> shading_model;
	DirectX::XMMATRIX world_transform;
	std::shared_ptr<unsigned char> custom_data;
	std::shared_ptr<D3DHelper::Device> m_device;
	bool b_render_resources_inited;

	virtual void CommitParams(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, D3DHelper::StaticDescriptorStack* param_stack) const = 0;
	virtual void InitRenderResources(std::shared_ptr<D3DHelper::Device> device)=0;
	virtual void UploadStatic(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list)=0;
	virtual void UploadDynamic(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,int game_frame) = 0;
	virtual int GetVertexCountPerInstance() const =0;
};

struct TriangleDeviceStaticResource;
class RENDER_MODULE_API TriangleRenderProxy:public RenderProxy
{
public:
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};
public:
	std::vector<Vertex> vertex;
	std::shared_ptr<TriangleDeviceStaticResource> device_static_resource;
	virtual void CommitParams(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, D3DHelper::StaticDescriptorStack* param_stack) const;
	virtual void InitRenderResources(std::shared_ptr<D3DHelper::Device> device);
	virtual void UploadStatic(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list);
	virtual void UploadDynamic(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,int game_frame);
	virtual int GetVertexCountPerInstance() const;
};

struct GaussianDeviceStaticResource;
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

public:
	std::vector<GaussianPoint> points_buffer;
	std::vector<GaussianCluster> clusters_buffer;
	std::shared_ptr<GaussianDeviceStaticResource> device_static_resource;
	std::shared_ptr<GaussianDeviceStaticResource> device_upload_resource;
	virtual void CommitParams(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list, D3DHelper::StaticDescriptorStack* param_stack) const;
	virtual void InitRenderResources(std::shared_ptr<D3DHelper::Device> device);
	virtual void UploadStatic(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list);
	virtual void UploadDynamic(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,int game_frame) {};
	virtual int GetVertexCountPerInstance() const;
};