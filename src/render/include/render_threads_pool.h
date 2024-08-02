#include<functional>
#include<vector>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>

#include"render_thread.h"
class RenderThreadsPool
{
private:
	std::vector<std::function<void>> m_task_queue;//todo lock free queue
	std::vector<RenderThread> m_threads;
public:
	static RenderThreadsPool* GetInst();
	void EnqueueRenderTask(std::function<void> func);
};