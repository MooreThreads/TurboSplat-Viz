#include"camera.h"
#include "world.h"
#include<cmath>

Camera::Camera(std::shared_ptr<World> world, float fov,float near_z,float far_z):SceneObject(world),fov(fov),near_z(near_z),far_z(far_z)
{
	m_controller = std::make_shared<Controller>(world);
}

Camera::Camera(std::shared_ptr<World> world, float fov, float near_z, float far_z,DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation) :SceneObject(world),
fov(fov), near_z(near_z), far_z(far_z)
{
	this->position = position;
	this->rotation = rotation;
	m_controller = std::make_shared<Controller>(world);
}

std::shared_ptr<RenderProxy> Camera::CreateRenderProxy()
{
	return nullptr;
}

void Camera::tick()
{
	//rotation.y += 0.01f;
}

void Camera::Init()
{
	SceneObject::Init();
	if (m_controller)
	{
		m_controller->SetOwner(std::dynamic_pointer_cast<SceneObject>(shared_from_this()));
		m_controller->Init();
	}
}

DirectX::XMMATRIX Camera::GetViewMatrix()
{
	DirectX::XMMATRIX camera_rot = DirectX::XMMatrixRotationRollPitchYaw(rotation.x / 180 * DirectX::XM_PI, rotation.y / 180 * DirectX::XM_PI, rotation.z / 180 * DirectX::XM_PI);
	camera_rot.r[3].m128_f32[0] = position.x;
	camera_rot.r[3].m128_f32[1] = position.y;
	camera_rot.r[3].m128_f32[2] = position.z;
	auto view_matrix=DirectX::XMMatrixInverse(nullptr,camera_rot);

	return view_matrix;
}
DirectX::XMFLOAT2 Camera::GetFocal(int viewport_width, int viewport_height)
{
	float half_fov_x = fov / 2 / 180 * DirectX::XM_PI;
	float tan_half_fov_x = std::tanf(half_fov_x);
	float tan_half_fov_y = tan_half_fov_x / viewport_width * viewport_height;
	float focal_x = viewport_width / (tan_half_fov_x * 2);
	float focal_y = viewport_height / (tan_half_fov_y * 2);
	return { focal_x,focal_y };
}
DirectX::XMMATRIX Camera::GetProjectMatrix(int viewport_width, int viewport_height,bool reverse_z)
{
	float half_fov_x = fov / 2 / 180 * DirectX::XM_PI;
	float tan_half_fov_x = std::tanf(half_fov_x);
	float tan_half_fov_y = tan_half_fov_x / viewport_width * viewport_height;

	float A, B;
	if (reverse_z)
	{
		A = near_z / (near_z - far_z);
		B = -far_z* near_z / (near_z - far_z);
	}
	else
	{
		A = far_z / (far_z - near_z);
		B = -far_z*near_z / (far_z - near_z);
	}

	DirectX::XMMATRIX ret(
		1.0f/ tan_half_fov_x, 0, 0, 0,
		0, 1.0f / tan_half_fov_y, 0, 0,
		0, 0, A, 1,
		0, 0, B, 0
	);

	return ret;
}