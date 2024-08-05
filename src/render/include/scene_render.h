#pragma once
#include "view_info.h"
#include <vector>
#include<memory>
#include "d3d_resources.h"
#include "dll_helper.h"

class RENDER_MODULE_API SceneRender
{
private:
	std::vector<ViewInfo> m_views;
	std::shared_ptr<D3dResources> m_d3dresource;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
public:
	SceneRender();
	void render(int game_frame);
};