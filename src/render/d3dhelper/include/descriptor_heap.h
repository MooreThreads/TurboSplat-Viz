#pragma once
#include <wrl.h>
#include <d3d12.h>
#include "d3dx12.h"
#include "device.h"
#include <memory>

namespace D3DHelper
{
	class Device;
	class StaticDescriptorHeap
	{
	protected:
		UINT m_descriptor_size;
		std::shared_ptr<Device> m_device;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap;
	public:
		StaticDescriptorHeap(std::shared_ptr<Device> device=nullptr);
		void Init(D3D12_DESCRIPTOR_HEAP_DESC desc, std::shared_ptr<Device> device=nullptr);
		CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPU(int index) const;
		CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPU(int index) const;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetHeap() const;
		int GetDescriptorNum() const;
		CD3DX12_CPU_DESCRIPTOR_HANDLE operator[](int index) const;
	};

	class StaticDescriptorStack:public StaticDescriptorHeap
	{
	protected:
		int top;
	public:
		StaticDescriptorStack(std::shared_ptr<Device> device = nullptr);
		void ResetStack();
		void push_back(const StaticDescriptorHeap& src,int offset, int num);
		CD3DX12_GPU_DESCRIPTOR_HANDLE GetTopGPU() const;
	};
}