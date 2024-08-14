#pragma once
#include<vector>
#include"view_info.h"
struct ViewportInfo
{
	std::vector<ViewInfo> views;
	HWND hwnd;
	int height;
	int width;
};

//Microsoft::WRL::ComPtr<IDXGISwapChain3> swap_chain;
//Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
