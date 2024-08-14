#pragma once
#include"d3d_resources.h"
#include<thread>
#include <wrl.h>
#include <d3d12.h>
#include<atomic>
#include <functional>
typedef std::function<void(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>)> RenderTask;

class RenderThreadsPool;
class RenderThread
{
private:
	std::thread m_thread;
	std::weak_ptr<RenderThreadsPool> m_threadpool;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_command_allocator[D3dResources::SWAPCHAIN_BUFFERCOUNT];
	int m_cur_allocator_index;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_command_list;
	const Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_command_queue_ref;
	std::atomic<bool> m_stop;

	void FrameInit();
	void FrameFinish();
	bool WaitTaskQueueReady();
	void ThreadRun_Internel();
public:
	RenderThread(Microsoft::WRL::ComPtr<ID3D12CommandQueue> command_queue,std::shared_ptr<RenderThreadsPool> threadpool);
	RenderThread(RenderThread&& other)noexcept;
	~RenderThread();
	void Run();
	void Stop();
	void Join();
};