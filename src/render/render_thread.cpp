#include"render_thread.h"
#include"render_threads_pool.h"
#include"d3d_helper.h"

RenderThread::RenderThread(std::shared_ptr<D3dResources> resource):m_d3dresource(resource)
{
	//init allocator and commmind list
	for(int i=0;i<D3dResources::kBufferCount;i++)
		ThrowIfFailed(m_d3dresource->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_command_allocator[i])));
	ThrowIfFailed(m_d3dresource->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_command_allocator[0].Get(), nullptr, IID_PPV_ARGS(&m_command_list)));
}