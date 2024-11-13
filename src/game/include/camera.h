#pragma once
#include<DirectXMath.h>
#include"object.h"
#include"controller.h"
class Camera:public SceneObject
{
protected:
	float fov;
	float near_z;
	float far_z;
	virtual std::shared_ptr<RenderProxy> CreateRenderProxy();
	std::shared_ptr<Controller> m_controller;
public:
	Camera(std::shared_ptr<World> world, float fov, float near_z, float far_z);
	Camera(std::shared_ptr<World> world, float fov, float near_z, float far_z, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation);
	virtual void Init();
	DirectX::XMMATRIX GetViewMatrix();
	DirectX::XMMATRIX GetProjectMatrix(int viewport_width,int viewport_height,bool reverse_z=false);
	float GetFov() const { return fov; }
	DirectX::XMFLOAT2 GetFocal(int viewport_width, int viewport_height);
	virtual void tick();
};