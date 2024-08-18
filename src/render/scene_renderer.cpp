#include "scene_renderer.h"
#include "render_threads_pool.h"
#include "d3d_resources.h"
#include "d3d_helper.h"
#include "render_threads_pool.h"
#include "render_proxy.h"
#include "shading_model.h"

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

void SceneRenderer::RenderObjInternel(const std::shared_ptr<RenderProxy>& proxy, const ViewInfo& view, int game_frame)
{
    m_render_threadspool->EnqueueRenderTask([proxy, view, game_frame](Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list) {
        if (proxy->b_render_resources_inited == false)
        {
            proxy->InitRenderResources();
            proxy->UploadStatic();
        }
        //view
        command_list->RSSetViewports(1, &view.m_viewport);
        command_list->RSSetScissorRects(1, &view.m_scissor_rect);
        command_list->OMSetRenderTargets(1, &view.render_target_view, FALSE, nullptr);

        //IA
        proxy->PopulateCommandList(command_list,view, game_frame % D3dResources::SWAPCHAIN_BUFFERCOUNT);

        return;
        });
}

void SceneRenderer::RenderViewInternel(Scene& scene, const ViewInfo& view,int game_frame)
{

    for (std::shared_ptr<RenderProxy> proxy_ptr : scene.GetRenderProxy())
    {
        RenderObjInternel(proxy_ptr, view, game_frame);
    }

    for (std::shared_ptr<RenderProxy> proxy_ptr : scene.GetTemproalRenderProxy())
    {
        RenderObjInternel(proxy_ptr, view, game_frame);
    }
}

void SceneRenderer::FrameInitCPU(int game_frame)
{

}
void SceneRenderer::FrameFinishCPU(int game_frame)
{
    //change rendertarget state
    ThrowIfFailed(m_command_allocator[game_frame % D3dResources::SWAPCHAIN_BUFFERCOUNT]->Reset());
}
void SceneRenderer::FrameInitCommandQueue(int game_frame)
{
    ThrowIfFailed(m_command_list->Reset(m_command_allocator[game_frame % D3dResources::SWAPCHAIN_BUFFERCOUNT].Get(), nullptr));
    m_command_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_swap_chain_buffer[game_frame%D3dResources::SWAPCHAIN_BUFFERCOUNT].Get(), 
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_command_list->ClearRenderTargetView(m_rtv_heap[game_frame % D3dResources::SWAPCHAIN_BUFFERCOUNT], clearColor, 0, nullptr);
    m_command_list->Close();
    ID3D12CommandList* ppCommandLists[] = { m_command_list.Get() };
    m_command_queue->ExecuteCommandLists(1, ppCommandLists);

}
void SceneRenderer::FrameFinishCommandQueue(int game_frame)
{
    ThrowIfFailed(m_command_list->Reset(m_command_allocator[game_frame % D3dResources::SWAPCHAIN_BUFFERCOUNT].Get(), nullptr));
    m_command_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_swap_chain_buffer[game_frame % D3dResources::SWAPCHAIN_BUFFERCOUNT].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
    m_command_list->Close();
    ID3D12CommandList* ppCommandLists[] = { m_command_list.Get() };
    m_command_queue->ExecuteCommandLists(1, ppCommandLists);
}

void SceneRenderer::Render(int game_frame)
{
    //render views t frame
    FrameInitCPU(game_frame);
    for (auto& view : m_viewport_info.views)
    {
        if (view.render_target_view.ptr == 0)
        {
            view.render_target_view= m_rtv_heap[game_frame % D3dResources::SWAPCHAIN_BUFFERCOUNT];
        }
        RenderViewInternel(m_scene_buffer[game_frame % D3dResources::SWAPCHAIN_BUFFERCOUNT], view,game_frame);
    }

    if (game_frame > 0)//wait t-1 frame task queue finish & set fence
    {
        m_render_threadspool->WaitRenderThreadFinish();
        FrameFinishCommandQueue(game_frame-1);
        ThrowIfFailed(m_command_queue->Signal(m_fence.Get(), game_frame - 1));
        ThrowIfFailed(m_fence->SetEventOnCompletion(game_frame - 1, m_fence_event[(game_frame - 1) % D3dResources::SWAPCHAIN_BUFFERCOUNT]));
        ThrowIfFailed(m_swap_chain->Present(0, 0));
    }
    if (game_frame > 1)//wait t2 frame present
    {
        GPU_SYNC(game_frame - 2);
        FrameFinishCPU(game_frame-2);
    }

    FrameInitCommandQueue(game_frame);
    m_render_threadspool->SwapTaskQueue();


}



void SceneRenderer::InitThreadsPool()
{
    m_render_threadspool = std::make_shared<RenderThreadsPool>();
    m_render_threadspool->Init(THREADS_NUM,m_command_queue);
    return;
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

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swap_chain;
    ThrowIfFailed(D3dResources::GetFactory()->CreateSwapChainForHwnd(
        m_command_queue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
        m_viewport_info.hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swap_chain
    ));
    ThrowIfFailed(swap_chain.As(&m_swap_chain));
}

void SceneRenderer::InitSwapChainBuffer()
{
    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = D3dResources::SWAPCHAIN_BUFFERCOUNT;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        m_rtv_heap.Init(rtvHeapDesc);
    }

    for (int i = 0; i < D3dResources::SWAPCHAIN_BUFFERCOUNT; i++)
    {
        ThrowIfFailed(m_swap_chain->GetBuffer(i, IID_PPV_ARGS(&m_swap_chain_buffer[i])));
        D3dResources::GetDevice()->CreateRenderTargetView(m_swap_chain_buffer[i].Get(), nullptr, m_rtv_heap[i]);
    }

    m_back_buffer_offset = m_swap_chain->GetCurrentBackBufferIndex();
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

    for (int i = 0; i < D3dResources::SWAPCHAIN_BUFFERCOUNT; i++)
        ThrowIfFailed(D3dResources::GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_command_allocator[i])));
    ThrowIfFailed(D3dResources::GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_command_allocator[0].Get(), nullptr, IID_PPV_ARGS(&m_command_list)));
    ThrowIfFailed(m_command_list->Close());

    InitSwapChainBuffer();
}

static void SetDefaultView(ViewInfo& view,int width,int height)
{
    view.view_matrix=DirectX::XMMATRIX(
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    );
    //near=1 far=100
    view.project_matrix = DirectX::XMMATRIX(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );
    view.m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    view.m_scissor_rect = CD3DX12_RECT(0, 0, static_cast<LONG>(width), static_cast<LONG>(height));
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

    //copy views
    m_viewport_info.views = viewport_info.views;
    if (m_viewport_info.views.empty())
    {
        //add default view
        ViewInfo view;
        SetDefaultView(view, m_viewport_info.width, m_viewport_info.height);
        view.render_target_view = m_rtv_heap[game_frame % D3dResources::SWAPCHAIN_BUFFERCOUNT];
        m_viewport_info.views.push_back(view);
    }
    
}

SceneRenderer::~SceneRenderer()
{
    m_render_threadspool->Close();
    return;
}