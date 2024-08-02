#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>

class D3dResources
{
private:
	//d3d resource
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D12Device> m_device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
public:
	static const int kBufferCount=3;

	D3dResources(HWND hwnd,int h,int w);
	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() { return m_device; }
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() { return m_commandQueue; }
};

