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
static void ExtractPitchYawRollFromXMMatrix(float* flt_p_PitchOut, float* flt_p_YawOut, float* flt_p_RollOut, const DirectX::XMMATRIX* XMMatrix_p_Rotation)
{
	DirectX::XMFLOAT4X4 XMFLOAT4X4_Values;
	DirectX::XMStoreFloat4x4(&XMFLOAT4X4_Values, DirectX::XMMatrixTranspose(*XMMatrix_p_Rotation));
	*flt_p_PitchOut = (float)asin(-XMFLOAT4X4_Values._23) / DirectX::XM_PI * 180;
	*flt_p_YawOut = (float)atan2(XMFLOAT4X4_Values._13, XMFLOAT4X4_Values._33) / DirectX::XM_PI * 180;
	*flt_p_RollOut = (float)atan2(XMFLOAT4X4_Values._21, XMFLOAT4X4_Values._22) / DirectX::XM_PI * 180;
}

void Controller::tick()
{
	auto owner = m_owner.lock();
	if (owner)
	{
		DirectX::XMFLOAT3 movement = DefaultInput::GetInst().GetMovement();
		auto OwnerRotMatrix=DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMVECTOR{ owner->rotation.x/180*DirectX::XM_PI, owner->rotation.y / 180 * DirectX::XM_PI, owner->rotation.z / 180 * DirectX::XM_PI, 1 });
		auto trans=DirectX::XMVector3Transform({ movement.x,movement.y,movement.z,1 }, OwnerRotMatrix);
		owner->position.x += 0.1f * trans.m128_f32[0];
		owner->position.y += 0.1f * trans.m128_f32[1];
		owner->position.z += 0.1f * trans.m128_f32[2];

		DirectX::XMINT3 turn = DefaultInput::GetInst().GetTurn();
		if (turn.x !=0|| turn.y != 0||turn.z!=0)
		{
			auto RotMatrix = DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMVECTOR{ -turn.y / 18000.0f * DirectX::XM_PI, turn.x / 18000.0f * DirectX::XM_PI,turn.z / 1800.0f * DirectX::XM_PI, 1 });
			auto NewOwnerRotMatrix = DirectX::XMMatrixMultiply(RotMatrix,OwnerRotMatrix);
			ExtractPitchYawRollFromXMMatrix(&owner->rotation.x, &owner->rotation.y, &owner->rotation.z, &NewOwnerRotMatrix);
		}

	}
	else
	{
		Erase();
	}
}