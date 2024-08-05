#pragma once
#include"d3d_resources.h"
#include<thread>
#include <wrl.h>
#include <d3d12.h>
#include<atomic>
#include <functional>
typedef std::function<void(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>, Microsoft::WRL::ComPtr<ID3D12CommandAllocator>)> RenderTask;

class RenderThreadsPool;
class RenderThread
{
private:
	std::thread m_thread;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_command_allocator[D3dResources::kBufferCount];
	int m_cur_allocator_index;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_command_list;
	std::shared_ptr<D3dResources> m_d3dresource;
	std::atomic<bool> m_stop;

	void FrameInit();
	void FrameFinish();
	void WaitTaskQueueReady();
	void ThreadRun_Internel();
public:
	RenderThread(std::shared_ptr<D3dResources> resource);
	RenderThread(RenderThread&& other)noexcept;
	~RenderThread();
	void Run();
	void Stop();
};