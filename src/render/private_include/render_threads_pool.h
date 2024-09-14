#pragma once
#include<functional>
#include<vector>
#include<queue>
#include<atomic>
#include<condition_variable>
#include<shared_mutex>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <set>
#include"render_thread.h"

namespace D3DHelper {
	class Device;
};

class RenderThreadsPool:public std::enable_shared_from_this< RenderThreadsPool>
{
private:
	std::queue<RenderTask> m_task_queue[2];//todo lock free queue
	std::vector<RenderThread> m_threads;

	std::set<std::thread::id> m_threadid_set;
	mutable std::shared_mutex mutex_threadid_set;
	
	int m_backqueue_index;
	bool b_init;

	std::shared_ptr<D3DHelper::Device> m_device;

public:
	RenderThreadsPool(std::shared_ptr<D3DHelper::Device> device);
	~RenderThreadsPool();
	std::mutex mutex;
	std::condition_variable renderthread_queue_ready;
	std::condition_variable renderthread_finished;
	std::atomic<int> ready_num;
	std::atomic<int> finish_num;

	int GetThreadsNum() const { return m_threads.size(); }
	bool IsRenderThread() const;
	void RegisterRenderThread(std::thread::id new_threadid);
	void UnregisterRenderThread(std::thread::id threadid);

	void Init(int threadsnum, Microsoft::WRL::ComPtr<ID3D12CommandQueue> command_queue);
	bool IsInitialized() const;
	void Close();

	void EnqueueRenderTask(RenderTask func);
	bool PopRenderTask(RenderTask& task);

	void WaitRenderThreadFinish();
	void SwapTaskQueue();
};