#include"controller.h"
#include"Keyboard.h"
#include"Mouse.h"
#include"device_input.h"
Controller::Controller(std::shared_ptr<World> world):Object(world)
{

}
void Controller::SetOwner(std::shared_ptr<SceneObject> owner)
{
	m_owner = owner;
}
void Controller::tick()
{
	auto owner = m_owner.lock();
	if (owner)
	{
		DirectX::XMFLOAT3 movement = DefaultInput::GetInst().GetMovement();
		owner->position.x += 0.1f * movement.x;
		owner->position.y += 0.1f * movement.y;
		owner->position.z += 0.1f * movement.z;
	}
	else
	{
		Erase();
	}
}