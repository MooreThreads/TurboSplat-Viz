#include"descriptor_heap.h"
#include "device.h"
#include"d3d_helper.h"
#include"assert.h"
using namespace D3DHelper;

StaticDescriptorHeap::StaticDescriptorHeap(std::shared_ptr<Device> device):m_descriptor_size(0), m_device(device), m_heap()
{

}
void StaticDescriptorHeap::Init(D3D12_DESCRIPTOR_HEAP_DESC desc, std::shared_ptr<Device> device)
{
	if (device)
	{
		m_device = device;
	}
	assert(m_device);
	m_heap.Reset();
	ThrowIfFailed(m_device->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_heap)));
	m_descriptor_size = m_device->GetDevice()->GetDescriptorHandleIncrementSize(desc.Type);

}
CD3DX12_CPU_DESCRIPTOR_HANDLE StaticDescriptorHeap::GetCPU(int index) const
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE ret(m_heap->GetCPUDescriptorHandleForHeapStart(), index, m_descriptor_size);
	return ret;
}
CD3DX12_GPU_DESCRIPTOR_HANDLE StaticDescriptorHeap::GetGPU(int index) const
{
	CD3DX12_GPU_DESCRIPTOR_HANDLE ret(m_heap->GetGPUDescriptorHandleForHeapStart(), index, m_descriptor_size);
	return ret;
}
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> StaticDescriptorHeap::GetHeap() const
{
	return m_heap;
}
int StaticDescriptorHeap::GetDescriptorNum() const
{
	return m_heap->GetDesc().NumDescriptors;
}
CD3DX12_CPU_DESCRIPTOR_HANDLE StaticDescriptorHeap::operator[](int index) const
{
	return GetCPU(index);
}

StaticDescriptorStack::StaticDescriptorStack(std::shared_ptr<Device> device):StaticDescriptorHeap(device),top(0)
{

}
void StaticDescriptorStack::ResetStack()
{
	top = 0;
}
void StaticDescriptorStack::push_back(const StaticDescriptorHeap& src, int offset, int num)
{
	assert(m_heap->GetDesc().Type == src.GetHeap()->GetDesc().Type);
	assert(top + num < m_heap->GetDesc().NumDescriptors);

	m_device->GetDevice()->CopyDescriptorsSimple(num, this->GetCPU(top), src.GetCPU(offset), m_heap->GetDesc().Type);
	top += num;
}
CD3DX12_GPU_DESCRIPTOR_HANDLE StaticDescriptorStack::GetTopGPU() const
{
	return GetGPU(top);
}