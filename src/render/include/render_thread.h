#include"d3d_resources.h"
#include<thread>
#include <wrl.h>
#include <d3d12.h>

class RenderThreadsPool;
class RenderThread
{
private:
	std::thread m_thread;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_command_allocator[D3dResources::kBufferCount];
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_command_list;
	std::weak_ptr< RenderThreadsPool> m_thread_pool;
	std::shared_ptr<D3dResources> m_d3dresource;
public:
	RenderThread(std::shared_ptr<D3dResources> resource);
	void run();
};