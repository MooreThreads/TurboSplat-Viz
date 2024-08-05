#include "scene_render.h"
#include "render_threads_pool.h"
#include "d3d_helper.h"

SceneRender::SceneRender():m_views()
{
	m_d3dresource = RenderThreadsPool::GetInst()->d3d_resource;
	ThrowIfFailed(m_d3dresource->GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	return;
}

void SceneRender::render(int game_frame)
{
	auto thread_pool=RenderThreadsPool::GetInst();
	if (game_frame != 0)
	{
		thread_pool->WaitRenderThreadFinish();
	}
	//create dx fence game_frame-1
	//wait dx fence game_frame-2
	thread_pool->SwapTaskQueue();//awake render threads
}