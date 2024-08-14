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
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_command_queue;

	void InitD3dResource();
	void CreateSwapChain();
	void InitThreadsPool();

	virtual void GPU_SYNC(int game_frame);

	virtual void RenderViewInternel(Scene& scene, const ViewInfo& view);
	virtual void RenderObjInternel(const std::shared_ptr<RenderProxy>& proxy, const ViewInfo& view);
public:
	SceneRenderer(ViewportInfo viewport_infos);
	virtual void Render(int game_frame);
	void Update(const Scene& game_scene ,ViewportInfo viewport_info,int game_frame);
};

