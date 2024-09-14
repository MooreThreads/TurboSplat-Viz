#pragma once
#include<thread>
#include <wrl.h>
#include <d3d12.h>
#include<atomic>
#include <functional>
#include "render_config.h"
#include "device.h"
#include "descriptor_heap.h"
typedef std::function<void(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>, D3DHelper::StaticDescriptorStack (& binded_heaps)[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES])> RenderTask;

class RenderThreadsPool;

struct RenderThreadFrameResource
{
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> command_allocator;
	D3DHelper::StaticDescriptorStack binded_heaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
};

class RenderThread
{
private:
	std::thread m_thread;
	std::weak_ptr<RenderThreadsPool> m_threadpool;
	RenderThreadFrameResource m_frame_resources[FRAME_BUFFER_COUNT];
	
	int m_cur_allocator_index;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_command_list;
	const Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_command_queue_ref;
	std::atomic<bool> m_stop;

	void FrameInit();
	void FrameFinish();
	bool WaitTaskQueueReady();
	void ThreadRun_Internel();
public:
	RenderThread(Microsoft::WRL::ComPtr<ID3D12CommandQueue> command_queue,std::shared_ptr<RenderThreadsPool> threadpool,std::shared_ptr<D3DHelper::Device> device);
	RenderThread(RenderThread&& other)noexcept;
	~RenderThread();
	void Run();
	void Stop();
	void Join();
};