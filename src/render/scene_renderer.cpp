#include "scene_renderer.h"
#include "render_threads_pool.h"
#include "d3d_resources.h"
#include "d3d_helper.h"
#include "render_threads_pool.h"

SceneRenderer::SceneRenderer(ViewportInfo viewport_infos):m_viewport_info(viewport_infos), m_scene_buffer{0,0,0}
{
    InitD3dResource();
    InitThreadsPool();
	return;
}


void SceneRenderer::GPU_SYNC(int game_frame)
{
    int ret_val=WaitForSingleObject(m_fence_event[game_frame % D3dResources::SWAPCHAIN_BUFFERCOUNT], INFINITE);
    int cur_fence_value = m_fence->GetCompletedValue();
    assert(cur_fence_value >= game_frame);
}

void SceneRenderer::RenderObjInternel(const std::shared_ptr<RenderProxy>& proxy, const ViewInfo& view)
{
    m_render_threadspool->EnqueueRenderTask([proxy, view](Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>) {

        });
}

void SceneRenderer::RenderViewInternel(Scene& scene, const ViewInfo& view)
{
    for (std::shared_ptr<RenderProxy> proxy_ptr : scene.GetRenderProxy())
    {
        RenderObjInternel(proxy_ptr, view);
    }

    for (std::shared_ptr<RenderProxy> proxy_ptr : scene.GetTemproalRenderProxy())
    {
        RenderObjInternel(proxy_ptr, view);
    }
}

void SceneRenderer::Render(int game_frame)
{
    //render views
    for (auto& view : m_viewport_info.views)
    {
        RenderViewInternel(m_scene_buffer[game_frame % D3dResources::SWAPCHAIN_BUFFERCOUNT], view);
    }

    if (game_frame > 0)//wait t-1 task queue finish & set fence
    {
        m_render_threadspool->WaitRenderThreadFinish();
        ThrowIfFailed(m_command_queue->Signal(m_fence.Get(), game_frame - 1));
        ThrowIfFailed(m_fence->SetEventOnCompletion(game_frame - 1, m_fence_event[(game_frame - 1) % D3dResources::SWAPCHAIN_BUFFERCOUNT]));
    }
    if (game_frame > 1)
    {
        ThrowIfFailed(m_swap_chain->Present(1, 0));
        GPU_SYNC(game_frame - 2);
    }
    m_render_threadspool->SwapTaskQueue();


}



void SceneRenderer::InitThreadsPool()
{
    m_render_threadspool = std::make_shared<RenderThreadsPool>();
    m_render_threadspool->Init(THREADS_NUM,m_command_queue);
}

void SceneRenderer::CreateSwapChain()
{
    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = D3dResources::SWAPCHAIN_BUFFERCOUNT;
    swapChainDesc.Width = m_viewport_info.width;
    swapChainDesc.Height = m_viewport_info.height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFailed(D3dResources::GetFactory()->CreateSwapChainForHwnd(
        m_command_queue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
        m_viewport_info.hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
    ));
    ThrowIfFailed(swapChain.As(&m_swap_chain));
}


void SceneRenderer::InitD3dResource()
{
    assert(m_viewport_info.hwnd != 0);
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(D3dResources::GetDevice()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_command_queue)));
    CreateSwapChain();
    ThrowIfFailed(D3dResources::GetFactory()->MakeWindowAssociation(m_viewport_info.hwnd, DXGI_MWA_NO_ALT_ENTER));// not support fullscreen transitions.
    ThrowIfFailed(D3dResources::GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
    for (int i = 0; i < D3dResources::SWAPCHAIN_BUFFERCOUNT; i++)
    {
        m_fence_event[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fence_event[i] == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }
    }
}

void SceneRenderer::Update(const Scene& game_scene, ViewportInfo viewport_info, int game_frame)
{
    assert(m_viewport_info.hwnd == viewport_info.hwnd);
    if (m_viewport_info.width != viewport_info.width || m_viewport_info.height != viewport_info.height)
    {
        m_viewport_info.width = viewport_info.width;
        m_viewport_info.height = viewport_info.height;
        CreateSwapChain();
    }

    //copy scene to buffer
    int scene_index = game_frame % D3dResources::SWAPCHAIN_BUFFERCOUNT;
    m_scene_buffer[scene_index] = game_scene;

    //check view
    m_viewport_info.views = viewport_info.views;
    if (m_viewport_info.views.empty())
    {
        //add default view
    }
    
}