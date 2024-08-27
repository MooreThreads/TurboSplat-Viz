#pragma once
#include <vector>
#include <map>
#include<memory>
#include "viewport_info.h"
#include "d3d_resources.h"
#include "scene.h"

class RenderThreadsPool;
class SceneRenderer
{
private:
	Scene m_scene_buffer[D3dResources::SWAPCHAIN_BUFFERCOUNT];
	ViewportInfo m_viewport_info;

	//pool
	std::shared_ptr<RenderThreadsPool> m_render_threadspool;
	static const int THREADS_NUM = 1;

	//d3d resources
	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
	HANDLE m_fence_event[D3dResources::SWAPCHAIN_BUFFERCOUNT];
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swap_chain;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_swap_chain_buffer[D3dResources::SWAPCHAIN_BUFFERCOUNT];
	int m_back_buffer_offset;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_command_queue;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_command_list;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_command_allocator[D3dResources::SWAPCHAIN_BUFFERCOUNT];
	D3dDescriptorHeapHelper m_rtv_heap;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_depthstencil_buffer[D3dResources::SWAPCHAIN_BUFFERCOUNT];
	D3dDescriptorHeapHelper m_dsv_heap;
	//D3dDescriptorHeapHelper m_dsv_uav_heap;
	int m_cur_cpu_frame;

	void InitD3dResource();
	void CreateSwapChain();
	void InitThreadsPool();

	virtual void GPU_SYNC(int game_frame);

	virtual void RenderViewInternel(Scene& scene, const ViewInfo& view,int game_frame);
	virtual void RenderObjInternel(const std::shared_ptr<RenderProxy>& proxy, const ViewInfo& view, int game_frame);
	void InitBuffer();
	virtual void FrameInitCPU(int game_frame);
	virtual void FrameFinishCPU(int game_frame);
	virtual void FrameInitCommandQueue(int game_frame);
	virtual void FrameFinishCommandQueue(int game_frame);
public:
	SceneRenderer(ViewportInfo viewport_infos);
	~SceneRenderer();
	virtual void Render(int game_frame);
	void Update(const Scene& game_scene ,ViewportInfo viewport_info,int game_frame);
};

