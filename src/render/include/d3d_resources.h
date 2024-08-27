#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "d3dx12.h"

class D3dResources
{


private:
	Microsoft::WRL::ComPtr<ID3D12Device2> m_device;
	Microsoft::WRL::ComPtr<IDXGIFactory4> m_factory;
	Microsoft::WRL::ComPtr<IDXGIAdapter1> m_hardware_adapter;
	static D3dResources* GetInst();
public:
	static const int SWAPCHAIN_BUFFERCOUNT = 3;
	D3dResources();
	void Init();
	static Microsoft::WRL::ComPtr<ID3D12Device2> GetDevice();
	static Microsoft::WRL::ComPtr<IDXGIFactory4> GetFactory();
	static Microsoft::WRL::ComPtr<IDXGIAdapter1> GetHardwareAdapter();

};

class D3dDescriptorHeapHelper
{
private:
	UINT m_descriptor_size;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap;
public:
	D3dDescriptorHeapHelper();
	void Init(D3D12_DESCRIPTOR_HEAP_DESC desc);
	CD3DX12_CPU_DESCRIPTOR_HANDLE Get(int index);
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPU(int index);
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetHeap() { return m_heap; }
	CD3DX12_CPU_DESCRIPTOR_HANDLE operator[](int index);
};