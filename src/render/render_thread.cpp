#include"render_thread.h"
#include"render_threads_pool.h"
#include"d3d_helper.h"
#include<assert.h>

RenderThread::RenderThread(Microsoft::WRL::ComPtr<ID3D12CommandQueue> command_queue, std::shared_ptr<RenderThreadsPool> threadpool): 
	m_cur_allocator_index(0), m_stop(true),m_command_queue_ref(command_queue),m_threadpool(threadpool)
{
	auto device = D3dResources::GetDevice();
	//init allocator and commmind list
	for(int i=0;i<D3dResources::SWAPCHAIN_BUFFERCOUNT;i++)
		ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_command_allocator[i])));
	ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_command_allocator[0].Get(), nullptr, IID_PPV_ARGS(&m_command_list)));
	ThrowIfFailed(m_command_list->Close());
}


RenderThread::RenderThread(RenderThread&& other) noexcept
{
	m_cur_allocator_index = other.m_cur_allocator_index;
	m_stop = other.m_stop.load();
	for (int i = 0; i < D3dResources::SWAPCHAIN_BUFFERCOUNT; i++)
	{
		m_command_allocator[i] = other.m_command_allocator[i];
	}
	m_command_list = other.m_command_list;
}

RenderThread::~RenderThread()
{
	Stop();
	if(m_thread.joinable())
		m_thread.join();
}

bool RenderThread::WaitTaskQueueReady()
{
	std::shared_ptr<RenderThreadsPool> pool = m_threadpool.lock();
	if (pool)
	{
		std::unique_lock<std::mutex> lg(pool->mutex);
		while (pool->ready_num <= 0)
		{
			pool->renderthread_queue_ready.wait(lg);
		}
		pool->ready_num.fetch_add(-1);
		return true;
	}
	return false;
}

void RenderThread::FrameInit()
{
	ThrowIfFailed(m_command_allocator[m_cur_allocator_index]->Reset());
	ThrowIfFailed(m_command_list->Reset(m_command_allocator[m_cur_allocator_index].Get(), nullptr));
}

void RenderThread::FrameFinish()
{
	ThrowIfFailed(m_command_list->Close());
	ID3D12CommandList* ppCommandLists[] = { m_command_list.Get() };
	m_command_queue_ref->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	std::shared_ptr<RenderThreadsPool> pool = m_threadpool.lock();
	m_cur_allocator_index = (m_cur_allocator_index + 1) % D3dResources::SWAPCHAIN_BUFFERCOUNT;
	int result = pool->finish_num.fetch_add(1);
	if (result + 1 == pool->GetThreadsNum())
	{
		pool->renderthread_finished.notify_all();
	}

}

void RenderThread::Run()
{
	assert(m_stop == true);
	m_stop = false;
	m_thread = std::thread(&RenderThread::ThreadRun_Internel, this);
}

void RenderThread::Stop()
{
	m_stop = true;
}

void RenderThread::Join()
{
	m_thread.join();
}

void RenderThread::ThreadRun_Internel()
{
	//todo assert render thread

	std::shared_ptr<RenderThreadsPool> pool = m_threadpool.lock();
	if (pool == nullptr)
	{
		//!!!weak_ptr error when multithread!!!
		assert(false);
	}
	pool->RegisterRenderThread(std::this_thread::get_id());

	while (m_stop==false)
	{
		if (WaitTaskQueueReady()==false||m_stop == true)
		{
			break;
		}

		FrameInit();
		RenderTask task;
		while (pool->PopRenderTask(task)&& m_stop==false)
		{
			task(m_command_list);
		}
		FrameFinish();
	}

	pool->UnregisterRenderThread(std::this_thread::get_id());
}