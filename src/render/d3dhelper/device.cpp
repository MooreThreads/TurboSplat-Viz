#include "device.h"
#include"d3d_helper.h"
using namespace D3DHelper;

DeviceManager::DeviceManager()
{
    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif
    
    std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter>> adapters;

    Microsoft::WRL::ComPtr<IDXGIFactory6> dxgiFactory6;
    Microsoft::WRL::ComPtr<IDXGIAdapter>  dxgiAdapter;
    Microsoft::WRL::ComPtr<IDXGIAdapter4> dxgiAdapter4;

    UINT createFactoryFlags = 0;
#if defined( _DEBUG )
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    ThrowIfFailed(::CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory6)));

    for (UINT i = 0; dxgiFactory6->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&dxgiAdapter))!=DXGI_ERROR_NOT_FOUND ; ++i)
    {
        DXGI_ADAPTER_DESC desc;
        dxgiAdapter->GetDesc(&desc);
        Microsoft::WRL::ComPtr<ID3D12Device2> d3d_device;
        if (SUCCEEDED(D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d_device))))
        {
            ThrowIfFailed(dxgiAdapter.As(&dxgiAdapter4));
            m_devices.emplace_back(std::make_shared<Device>(dxgiAdapter4, d3d_device));
        }
    }


}

DeviceManager* DeviceManager::GetDeviceManager()
{
	static DeviceManager inst;
	return &inst;
}

std::shared_ptr<Device> DeviceManager::GetDefaultDevice()
{
    return m_devices[0];
}

Device::Device(Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter, Microsoft::WRL::ComPtr<ID3D12Device2> device):m_device(device),m_hardware_adapter(adapter)
{

}
Microsoft::WRL::ComPtr<ID3D12Device2> Device::GetDevice()
{
    return m_device;
}
Microsoft::WRL::ComPtr<IDXGIAdapter1> Device::GetHardwareAdapter()
{
    return m_hardware_adapter;
}

Microsoft::WRL::ComPtr<IDXGIFactory4> Device::GetFactory()
{
    Microsoft::WRL::ComPtr<IDXGIFactory>  factory;
    ThrowIfFailed(m_hardware_adapter->GetParent(IID_PPV_ARGS(&factory)));
    Microsoft::WRL::ComPtr<IDXGIFactory4> factory4;
    ThrowIfFailed(factory.As(&factory4));
    return factory4;
}