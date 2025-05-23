#pragma once
#include <vector>
#include <map>
#include<memory>
#include <dxgi1_6.h>
#include "viewport_info.h"
#include "scene.h"
#include "render_config.h"
#include "descriptor_heap.h"


class RenderThreadsPool;
namespace D3DHelper {
	class Device;
}


class SceneRenderer
{
private:
	Scene m_scene_buffer[FRAME_BUFFER_COUNT];
	ViewportInfo m_viewport_info;

	std::shared_ptr<D3DHelper::Device> m_device;

	//pool
	std::shared_ptr<RenderThreadsPool> m_render_threadspool;

	//d3d resources
	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
	HANDLE m_fence_event[FRAME_BUFFER_COUNT];
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swap_chain;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_swap_chain_buffer[FRAME_BUFFER_COUNT];
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_command_queue;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_command_list;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_command_allocator[FRAME_BUFFER_COUNT];
	D3DHelper::StaticDescriptorHeap m_rtv_heap;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_depthstencil_buffer[FRAME_BUFFER_COUNT];
	D3DHelper::StaticDescriptorHeap m_dsv_heap;

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
	SceneRenderer(ViewportInfo viewport_infos, std::shared_ptr<D3DHelper::Device> device);
	~SceneRenderer();
	virtual void Render(int game_frame);
	void Update(const Scene& game_scene ,ViewportInfo viewport_info,int game_frame);
};

