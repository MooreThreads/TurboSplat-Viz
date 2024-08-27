#pragma once
#include"DirectXMath.h"
#include <wrl.h>
#include <d3d12.h>
#include "d3dx12.h"
#include "scene.h"
struct ViewInfo
{
	DirectX::XMMATRIX view_matrix;
	DirectX::XMMATRIX project_matrix;
	DirectX::XMFLOAT2 focal;
	CD3DX12_CPU_DESCRIPTOR_HANDLE render_target_view;
	CD3DX12_CPU_DESCRIPTOR_HANDLE render_target_uav;
	CD3DX12_CPU_DESCRIPTOR_HANDLE depth_stencil_view;
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissor_rect;
};
