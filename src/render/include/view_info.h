#pragma once
#include"DirectXMath.h"
#include <wrl.h>
#include <d3d12.h>
#include "d3dx12.h"
#include "scene.h"
struct ViewInfo
{
	DirectX::FXMMATRIX view_matrix;
	DirectX::FXMMATRIX project_matrix;
	Microsoft::WRL::ComPtr<ID3D12Resource> render_target;
	CD3DX12_CPU_DESCRIPTOR_HANDLE render_target_view;
};
