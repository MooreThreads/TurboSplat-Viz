#include"render_threads_pool.h"
#include<assert.h>
void RenderThreadsPool::EnqueueRenderTask(RenderTask func)
{
	m_task_queue[m_backqueue_index].push(func);
}

bool RenderThreadsPool::PopRenderTask(RenderTask& task)
{
	assert(IsRenderThread());
	int front_queue_index = (m_backqueue_index + 1) % 2;
	if (m_task_queue[front_queue_index].empty() == false)
	{
		task = m_task_queue[front_queue_index].front();
		m_task_queue[front_queue_index].pop();
		return true;
	}
	return false;
}


RenderThreadsPool::RenderThreadsPool():m_task_queue(), m_threads(), m_backqueue_index(0), ready_num(0), finish_num(0),b_init(false), d3d_resource(nullptr)
{
}
RenderThreadsPool::~RenderThreadsPool()
{
	for (auto& thread:m_threads)
	{
		thread.Stop();
	}
	renderthread_queue_ready.notify_all();
	m_threads.clear();


}

bool RenderThreadsPool::IsInitialized() const
{
	return b_init;
}

void RenderThreadsPool::Init(int threadsnum, Microsoft::WRL::ComPtr<ID3D12CommandQueue> command_queue)
{
	assert(IsRenderThread()==false);

	if (b_init == true)
	{
		for (int i = 0; i < threadsnum; i++)
		{
			m_threads[i].Stop();
			m_threads[i].Join();
		}
		m_threads.clear();
	}

	assert(m_threads.size() == 0);
	for (int i = 0; i < threadsnum; i++)
	{
		m_threads.emplace_back(command_queue, shared_from_this());
	}
	for (int i = 0; i < threadsnum; i++)
	{
		m_threads[i].Run();
	}
	b_init = true;
}

bool RenderThreadsPool::IsRenderThread() const
{
	mutex_threadid_set.lock_shared();
	auto result = m_threadid_set.find(std::this_thread::get_id());
	mutex_threadid_set.unlock_shared();
	return result != m_threadid_set.end();
}
void RenderThreadsPool::RegisterRenderThread(std::thread::id new_threadid)
{
	mutex_threadid_set.lock();
	assert(m_threadid_set.find(new_threadid) == m_threadid_set.end());
	m_threadid_set.insert(new_threadid);
	mutex_threadid_set.unlock();
}
void RenderThreadsPool::UnregisterRenderThread(std::thread::id threadid)
{
	mutex_threadid_set.lock();
	assert(m_threadid_set.find(threadid) != m_threadid_set.end());
	m_threadid_set.erase(threadid);
	mutex_threadid_set.unlock();
}

void RenderThreadsPool::WaitRenderThreadFinish()
{
	assert(IsRenderThread() == false);
	assert(m_threads.size() != 0);

	std::unique_lock<std::mutex> lg(mutex);
	while (finish_num!= m_threads.size())
	{
		renderthread_finished.wait(lg);
	}
	finish_num = 0;

	int front_queue_index = (m_backqueue_index + 1) % 2;
	assert(m_task_queue[front_queue_index].empty());
}

void RenderThreadsPool::SwapTaskQueue()
{
	assert(IsRenderThread() == false);
	assert(m_threads.size() != 0);

	std::lock_guard<std::mutex> lockgard(mutex);
	m_backqueue_index = (m_backqueue_index + 1) % 2;
	assert(m_task_queue[m_backqueue_index].empty());
	ready_num = m_threads.size();
	renderthread_queue_ready.notify_all();
}