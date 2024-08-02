#include"render_threads_pool.h"

void RenderThreadsPool::EnqueueRenderTask(std::function<void> func)
{
	m_task_queue.push_back(func);
}
RenderThreadsPool* RenderThreadsPool::GetInst()
{
	static RenderThreadsPool inst;
	return &inst;
}