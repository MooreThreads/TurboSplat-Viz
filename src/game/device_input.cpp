#include"device_input.h"
DefaultInput::DefaultInput()
{

}
void DefaultInput::Init(std::unique_ptr<DirectX::Keyboard>&& keybaord, std::unique_ptr<DirectX::Mouse>&& mouse)
{
	m_keyboard = std::forward<std::unique_ptr<DirectX::Keyboard>>(keybaord);
	m_mouse = std::forward<std::unique_ptr<DirectX::Mouse>>(mouse);
}
DefaultInput& DefaultInput::GetInst()
{
	static DefaultInput inst;
	return inst;
}
DirectX::XMFLOAT3 DefaultInput::GetMovement()
{
	assert(m_keyboard);
	DirectX::XMFLOAT3 ret_val{ 0,0,0 };
	auto state = m_keyboard->GetState();
	if (state.A)
	{
		ret_val.x -= 1;
	}
	if (state.D)
	{
		ret_val.x += 1;
	}
	if (state.W)
	{
		ret_val.y += 1;
	}
	if (state.S)
	{
		ret_val.y -= 1;
	}
	if (state.Q)
	{
		ret_val.z -= 1;
	}
	if (state.E)
	{
		ret_val.z += 1;
	}
	return ret_val;
}
DirectX::XMFLOAT3 DefaultInput::GetTurn()
{
	DirectX::XMFLOAT3 ret_val{ 0,0,0 };
	return ret_val;
}