#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "d3dx12.h"

namespace D3DHelper
{
	class Device
	{
	private:
		Microsoft::WRL::ComPtr<ID3D12Device2> m_device;
		Microsoft::WRL::ComPtr<IDXGIAdapter1> m_hardware_adapter;
	public:
		Device(Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter, Microsoft::WRL::ComPtr<ID3D12Device2> device);
		Microsoft::WRL::ComPtr<ID3D12Device2> GetDevice();
		Microsoft::WRL::ComPtr<IDXGIAdapter1> GetHardwareAdapter();
		Microsoft::WRL::ComPtr<IDXGIFactory4> GetFactory();
	};


	class DeviceManager
	{
	private:
		std::vector<std::shared_ptr<Device>> m_devices;
	public:
		DeviceManager();
		static DeviceManager* GetDeviceManager();
		std::shared_ptr<Device> GetDefaultDevice();
	};
}