#include "scene_renderer.h"
#include "render_threads_pool.h"
#include "d3d_helper.h"
#include "render_threads_pool.h"
#include "render_proxy.h"
#include "shading_model.h"
#include "device.h"

SceneRenderer::SceneRenderer(ViewportInfo viewport_infos, std::shared_ptr<D3DHelper::Device> device):m_viewport_info(viewport_infos), m_scene_buffer{0,0,0},
    m_device(device), m_rtv_heap(device),m_dsv_heap(device)
{
    InitD3dResource();
    InitThreadsPool();
    ThrowIfFailed(m_swap_chain->Present(0, 0));//Skip frame 0 because fence_value 0 is invald for sync.
	return;
}


void SceneRenderer::GPU_SYNC(int game_frame)
{
    if (m_fence->GetCompletedValue() < game_frame)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(game_frame, m_fence_event[game_frame % FRAME_BUFFER_COUNT]));
        int ret_val = WaitForSingleObject(m_fence_event[game_frame % FRAME_BUFFER_COUNT], INFINITE);
        int cur_fence_value = m_fence->GetCompletedValue();
        assert(cur_fence_value >= game_frame);
    }
}

void SceneRenderer::RenderObjInternel(const std::shared_ptr<RenderProxy>& proxy, const ViewInfo& view, int game_frame)
{
    auto cur_device = m_device;
    m_render_threadspool->EnqueueRenderTask([proxy, view, game_frame, cur_device](Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
        D3DHelper::StaticDescriptorStack(&binded_heaps)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES]) {
        if (proxy->b_render_resources_inited == false)
        {
            proxy->InitRenderResources(cur_device);
            proxy->UploadStatic(command_list);
        }
        assert(proxy->shading_model);
        proxy->shading_model->PopulateCommandList(command_list, binded_heaps, game_frame % FRAME_BUFFER_COUNT,&view, proxy.get());
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
    ThrowIfFailed(m_command_allocator[game_frame % FRAME_BUFFER_COUNT]->Reset());
    return;
}
void SceneRenderer::FrameInitCommandQueue(int game_frame)
{
    ThrowIfFailed(m_command_list->Reset(m_command_allocator[game_frame % FRAME_BUFFER_COUNT].Get(), nullptr));
    m_command_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_swap_chain_buffer[game_frame%FRAME_BUFFER_COUNT].Get(), 
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    m_command_list->ClearRenderTargetView(m_rtv_heap[game_frame % FRAME_BUFFER_COUNT], clearColor, 0, nullptr);
    m_command_list->ClearDepthStencilView(m_dsv_heap[game_frame % FRAME_BUFFER_COUNT], D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    m_command_list->Close();
    ID3D12CommandList* ppCommandLists[] = { m_command_list.Get() };
    m_command_queue->ExecuteCommandLists(1, ppCommandLists);

}
void SceneRenderer::FrameFinishCommandQueue(int game_frame)
{
    ThrowIfFailed(m_command_list->Reset(m_command_allocator[game_frame % FRAME_BUFFER_COUNT].Get(), nullptr));
    m_command_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_swap_chain_buffer[game_frame % FRAME_BUFFER_COUNT].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
    m_command_list->Close();
    ID3D12CommandList* ppCommandLists[] = { m_command_list.Get() };
    m_command_queue->ExecuteCommandLists(1, ppCommandLists);
}

void SceneRenderer::Render(int game_frame)
{
    //render views t frame
    m_cur_cpu_frame = game_frame;
    FrameInitCPU(game_frame);

    //render
    for (auto& view : m_viewport_info.views)
    {
        if (view.render_target_view.ptr == 0)
        {
            view.render_target_view= m_rtv_heap[game_frame % FRAME_BUFFER_COUNT];
            view.render_target_buffer = m_swap_chain_buffer[game_frame % FRAME_BUFFER_COUNT];
            view.depth_stencil_view = m_dsv_heap[game_frame % FRAME_BUFFER_COUNT];
        }
        RenderViewInternel(m_scene_buffer[game_frame % FRAME_BUFFER_COUNT], view,game_frame);
    }

    if (game_frame > 1)//wait t-1 frame task queue finish & set fence
    {
        m_render_threadspool->WaitRenderThreadFinish();
        FrameFinishCommandQueue(game_frame-1);
        ThrowIfFailed(m_command_queue->Signal(m_fence.Get(), game_frame - 1));
        ThrowIfFailed(m_swap_chain->Present(0, 0));
    }
    if (game_frame > 2)//wait t2 frame present
    {
        GPU_SYNC(game_frame - 2);
        FrameFinishCPU(game_frame-2);
    }

    FrameInitCommandQueue(game_frame);
    m_render_threadspool->SwapTaskQueue();


}



void SceneRenderer::InitThreadsPool()
{
    m_render_threadspool = std::make_shared<RenderThreadsPool>(m_device);
    m_render_threadspool->Init(RENDERER_THREADS_NUM,m_command_queue);
    return;
}

void SceneRenderer::CreateSwapChain()
{
    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FRAME_BUFFER_COUNT;
    swapChainDesc.Width = m_viewport_info.width;
    swapChainDesc.Height = m_viewport_info.height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swap_chain;
    ThrowIfFailed(m_device->GetFactory()->CreateSwapChainForHwnd(
        m_command_queue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
        m_viewport_info.hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swap_chain
    ));
    ThrowIfFailed(swap_chain.As(&m_swap_chain));
}

void SceneRenderer::InitBuffer()
{
    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FRAME_BUFFER_COUNT;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        m_rtv_heap.Init(rtvHeapDesc);

        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {}; // Note: DepthStencil View requires storage in a heap even if we are going to use only 1 view
        dsvHeapDesc.NumDescriptors = FRAME_BUFFER_COUNT;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        m_dsv_heap.Init(dsvHeapDesc);
    }

    //create depth stencil buffer
    int width = m_viewport_info.width;
    int height = m_viewport_info.height;
    auto depth_stencil_buffer_desc=CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
    D3D12_CLEAR_VALUE clear_value;
    clear_value.Format = DXGI_FORMAT_D32_FLOAT;
    clear_value.DepthStencil.Depth = 1.0f;
    for (int i = 0; i < FRAME_BUFFER_COUNT; i++)
    {
        m_device->GetDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &depth_stencil_buffer_desc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE, &clear_value, IID_PPV_ARGS(&m_depthstencil_buffer[i]));
    }

    for (int i = 0; i < FRAME_BUFFER_COUNT; i++)
    {
        ThrowIfFailed(m_swap_chain->GetBuffer(i, IID_PPV_ARGS(&m_swap_chain_buffer[i])));
        m_device->GetDevice()->CreateRenderTargetView(m_swap_chain_buffer[i].Get(), nullptr, m_rtv_heap[i]);
        m_device->GetDevice()->CreateDepthStencilView(m_depthstencil_buffer[i].Get(), nullptr, m_dsv_heap[i]);
    }

}


void SceneRenderer::InitD3dResource()
{
    assert(m_viewport_info.hwnd != 0);
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(m_device->GetDevice()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_command_queue)));
    CreateSwapChain();
    ThrowIfFailed(m_device->GetFactory()->MakeWindowAssociation(m_viewport_info.hwnd, DXGI_MWA_NO_ALT_ENTER));// not support fullscreen transitions.
    ThrowIfFailed(m_device->GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
    m_fence->SetName(L"render fence");
    for (int i = 0; i < FRAME_BUFFER_COUNT; i++)
    {
        m_fence_event[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fence_event[i] == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }
    }

    for (int i = 0; i < FRAME_BUFFER_COUNT; i++)
    {
        ThrowIfFailed(m_device->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_command_allocator[i])));
        m_command_allocator[i]->SetName(L"SceneRenderCommandAllocator");
    }
    ThrowIfFailed(m_device->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_command_allocator[0].Get(), nullptr, IID_PPV_ARGS(&m_command_list)));
    ThrowIfFailed(m_command_list->Close());

    InitBuffer();
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
    int scene_index = game_frame % FRAME_BUFFER_COUNT;
    m_scene_buffer[scene_index] = game_scene;

    //copy views
    m_viewport_info.views = viewport_info.views;
    if (m_viewport_info.views.empty())
    {
        //add default view
        ViewInfo view;
        SetDefaultView(view, m_viewport_info.width, m_viewport_info.height);
        view.render_target_view = m_rtv_heap[game_frame % FRAME_BUFFER_COUNT];
        view.depth_stencil_view = m_dsv_heap[game_frame % FRAME_BUFFER_COUNT];
        m_viewport_info.views.push_back(view);
    }
    
}

SceneRenderer::~SceneRenderer()
{
    m_render_threadspool->WaitRenderThreadFinish();
    ThrowIfFailed(m_command_queue->Signal(m_fence.Get(), m_cur_cpu_frame));
    ThrowIfFailed(m_fence->SetEventOnCompletion(m_cur_cpu_frame, m_fence_event[m_cur_cpu_frame % FRAME_BUFFER_COUNT]));
    GPU_SYNC(m_cur_cpu_frame);
    m_render_threadspool->Close();
    return;
}