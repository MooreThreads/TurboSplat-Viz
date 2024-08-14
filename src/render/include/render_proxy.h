#pragma
#include<memory>
#include "render_dll_helper.h"
#include"DirectXMath.h"
class ShadingModel;
class RENDER_MODULE_API RenderProxy
{
private:
	virtual void InitRenderResources()=0;
public:
	std::shared_ptr<ShadingModel> shading_model;
	DirectX::FXMMATRIX world_transform;
	std::shared_ptr<unsigned char> custom_data;

	virtual void PopulateCommandList()=0;
};

class RENDER_MODULE_API ScreenTriangleRenderProxy:public RenderProxy
{
private:
	virtual void InitRenderResources();
public:
	virtual void PopulateCommandList();
};