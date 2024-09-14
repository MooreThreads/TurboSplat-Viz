#include"render_thread.h"
#include"render_threads_pool.h"
#include"d3d_helper.h"
#include<assert.h>
#include"device.h"

RenderThread::RenderThread(Microsoft::WRL::ComPtr<ID3D12CommandQueue> command_queue, std::shared_ptr<RenderThreadsPool> threadpool, std::shared_ptr<D3DHelper::Device> device):
	m_cur_allocator_index(0), m_stop(true),m_command_queue_ref(command_queue),m_threadpool(threadpool)
{
	//init allocator and commmind list
	for (int i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		ThrowIfFailed(device->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_frame_resources[i].command_allocator)));
		m_frame_resources[i].command_allocator->SetName(L"RenderThreadCommandAllocator");
		D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
		heap_desc.NumDescriptors = 1024;
		heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		m_frame_resources[i].binded_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].Init(heap_desc,device);
		heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		m_frame_resources[i].binded_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER].Init(heap_desc, device);

	}
	ThrowIfFailed(device->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_frame_resources[0].command_allocator.Get(), nullptr, IID_PPV_ARGS(&m_command_list)));
	ThrowIfFailed(m_command_list->Close());

}


RenderThread::RenderThread(RenderThread&& other) noexcept
{
	m_cur_allocator_index = other.m_cur_allocator_index;
	m_stop = other.m_stop.load();
	for (int i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		m_frame_resources[i] = other.m_frame_resources[i];
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
	ThrowIfFailed(m_frame_resources[m_cur_allocator_index].command_allocator->Reset());
	m_frame_resources[m_cur_allocator_index].binded_heaps[0].ResetStack();
	m_frame_resources[m_cur_allocator_index].binded_heaps[1].ResetStack();
	ThrowIfFailed(m_command_list->Reset(m_frame_resources[m_cur_allocator_index].command_allocator.Get(), nullptr));

	ID3D12DescriptorHeap* heaps_array[] = {m_frame_resources[m_cur_allocator_index].binded_heaps[0].GetHeap().Get(),
		m_frame_resources[m_cur_allocator_index].binded_heaps[1].GetHeap().Get()};
	m_command_list->SetDescriptorHeaps(2, heaps_array);
}

void RenderThread::FrameFinish()
{
	ThrowIfFailed(m_command_list->Close());
	ID3D12CommandList* ppCommandLists[] = { m_command_list.Get() };
	m_command_queue_ref->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	std::shared_ptr<RenderThreadsPool> pool = m_threadpool.lock();
	m_cur_allocator_index = (m_cur_allocator_index + 1) % FRAME_BUFFER_COUNT;
	{
		std::unique_lock<std::mutex> lg(pool->mutex);
		int result = pool->finish_num.fetch_add(1);
		if (result + 1 == pool->GetThreadsNum())
		{
			pool->renderthread_finished.notify_all();
		}
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
			task(m_command_list, m_frame_resources[m_cur_allocator_index].binded_heaps);
		}
		FrameFinish();
	}

	pool->UnregisterRenderThread(std::this_thread::get_id());
}