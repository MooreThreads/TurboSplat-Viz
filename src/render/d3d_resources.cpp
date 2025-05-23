/*

#include"d3d_resources.h"
#include"d3d_helper.h"
#include<assert.h>
using Microsoft::WRL::ComPtr;

static void GetHardwareAdapter(
    IDXGIFactory1* pFactory,
    IDXGIAdapter1** ppAdapter,
    bool requestHighPerformanceAdapter)
{
    *ppAdapter = nullptr;

    ComPtr<IDXGIAdapter1> adapter;

    ComPtr<IDXGIFactory6> factory6;
    if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
    {
        for (
            UINT adapterIndex = 0;
            SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                adapterIndex,
                requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                IID_PPV_ARGS(&adapter)));
            ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }

    if (adapter.Get() == nullptr)
    {
        for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }

    *ppAdapter = adapter.Detach();
}


DeviceManager::DeviceManager():m_device(nullptr),m_hardware_adapter(nullptr)
{

}

void DeviceManager::Init()
{
    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif
    ComPtr<IDXGIFactory2>  dxgiFactory;
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));
    ::GetHardwareAdapter(dxgiFactory.Get(), &m_hardware_adapter, false);

    ThrowIfFailed(D3D12CreateDevice(m_hardware_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device)));
}

DeviceManager* DeviceManager::GetInst()
{
    static DeviceManager inst;
    if (inst.m_device == nullptr)
    {
        inst.Init();
    }
    return &inst;
}

Microsoft::WRL::ComPtr<ID3D12Device2> DeviceManager::GetDevice()
{
    auto ret=GetInst()->m_device;
    assert(ret != nullptr);
    return ret;
}


Microsoft::WRL::ComPtr<IDXGIAdapter1> DeviceManager::GetHardwareAdapter()
{
    auto ret = GetInst()->m_hardware_adapter;
    assert(ret != nullptr);
    return ret;
}


D3dDescriptorHeapHelper::D3dDescriptorHeapHelper():m_descriptor_size(0)
{

}
void D3dDescriptorHeapHelper::Init(D3D12_DESCRIPTOR_HEAP_DESC desc)
{
    ThrowIfFailed(DeviceManager::GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_heap)));
    m_descriptor_size = DeviceManager::GetDevice()->GetDescriptorHandleIncrementSize(desc.Type);
}
CD3DX12_CPU_DESCRIPTOR_HANDLE D3dDescriptorHeapHelper::Get(int index)
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE ret(m_heap->GetCPUDescriptorHandleForHeapStart(), index, m_descriptor_size);
    return ret;
}

CD3DX12_GPU_DESCRIPTOR_HANDLE D3dDescriptorHeapHelper::GetGPU(int index)
{
    CD3DX12_GPU_DESCRIPTOR_HANDLE ret(m_heap->GetGPUDescriptorHandleForHeapStart(), index, m_descriptor_size);
    return ret;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE D3dDescriptorHeapHelper::operator[](int index)
{
    return Get(index);
}

*/