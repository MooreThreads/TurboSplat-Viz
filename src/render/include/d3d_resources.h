#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>

class D3dResources
{


private:
	Microsoft::WRL::ComPtr<ID3D12Device> m_device;
	Microsoft::WRL::ComPtr<IDXGIFactory4> m_factory;
	Microsoft::WRL::ComPtr<IDXGIAdapter1> m_hardware_adapter;
	static D3dResources* GetInst();
public:
	static const int SWAPCHAIN_BUFFERCOUNT = 3;
	D3dResources();
	void Init();
	static Microsoft::WRL::ComPtr<ID3D12Device> GetDevice();
	static Microsoft::WRL::ComPtr<IDXGIFactory4> GetFactory();
	static Microsoft::WRL::ComPtr<IDXGIAdapter1> GetHardwareAdapter();

};