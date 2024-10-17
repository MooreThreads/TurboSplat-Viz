#pragma once
#include<DirectXMath.h>
#include"object.h"
class Camera:public Object
{
protected:
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation;
	float fov;
	float near_z;
	float far_z;
public:
	Camera(std::shared_ptr<World> world, float fov, float near_z, float far_z);
	Camera(std::shared_ptr<World> world, float fov, float near_z, float far_z, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation);
	DirectX::XMMATRIX GetViewMatrix();
	DirectX::XMMATRIX GetProjectMatrix(int viewport_width,int viewport_height,bool reverse_z=false);
	float GetFov() const { return fov; }
	DirectX::XMFLOAT2 GetFocal(int viewport_width, int viewport_height);
	virtual void tick();
};