#include"render_thread.h"
#include"render_threads_pool.h"
#include"d3d_helper.h"
#include<assert.h>

RenderThread::RenderThread(std::shared_ptr<D3dResources> resource):m_d3dresource(resource), m_cur_allocator_index(0), m_stop(true)
{
	//init allocator and commmind list
	for(int i=0;i<D3dResources::kBufferCount;i++)
		ThrowIfFailed(m_d3dresource->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_command_allocator[i])));
	ThrowIfFailed(m_d3dresource->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_command_allocator[0].Get(), nullptr, IID_PPV_ARGS(&m_command_list)));
}


RenderThread::RenderThread(RenderThread&& other) noexcept
{
	m_d3dresource = other.m_d3dresource;
	m_cur_allocator_index = other.m_cur_allocator_index;
	m_stop = other.m_stop.load();
	for (int i = 0; i < D3dResources::kBufferCount; i++)
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

void RenderThread::WaitTaskQueueReady()
{
	RenderThreadsPool* pool = RenderThreadsPool::GetInst();
	std::unique_lock<std::mutex> lg(pool->mutex);
	while (pool->ready_num <= 0)
	{
		pool->renderthread_queue_ready.wait(lg);
	}
	pool->ready_num.fetch_add(-1);
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
	m_d3dresource->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	m_cur_allocator_index = (m_cur_allocator_index + 1) % D3dResources::kBufferCount;
	int result = RenderThreadsPool::GetInst()->finish_num.fetch_add(1);
	if (result + 1 == RenderThreadsPool::GetInst()->GetThreadsNum())
	{
		RenderThreadsPool::GetInst()->renderthread_finished.notify_all();
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

void RenderThread::ThreadRun_Internel()
{
	//todo assert render thread

	RenderThreadsPool* pool = RenderThreadsPool::GetInst();
	pool->RegisterRenderThread(std::this_thread::get_id());

	while (m_stop==false)
	{
		WaitTaskQueueReady();
		if (m_stop == true)
		{
			break;
		}

		FrameInit();
		RenderTask task;
		while (pool->PopRenderTask(task)&& m_stop==false)
		{
			task(m_command_list, m_command_allocator[m_cur_allocator_index]);
		}
		FrameFinish();
	}

	pool->UnregisterRenderThread(std::this_thread::get_id());
}