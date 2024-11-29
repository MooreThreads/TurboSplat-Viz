#include"device_input.h"
DefaultInput::DefaultInput()
{

}
void DefaultInput::Init(std::unique_ptr<DirectX::Keyboard>&& keybaord, std::unique_ptr<DirectX::Mouse>&& mouse,HWND window)
{
	m_keyboard = std::forward<std::unique_ptr<DirectX::Keyboard>>(keybaord);
	m_mouse = std::forward<std::unique_ptr<DirectX::Mouse>>(mouse);
	m_mouse->SetWindow(window);
	m_mouse->SetMode(DirectX::Mouse::MODE_RELATIVE);
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
		ret_val.z += 1;
	}
	if (state.S)
	{
		ret_val.z -= 1;
	}
	if (state.Space)
	{
		ret_val.y -= 1;
	}
	if (state.C)
	{
		ret_val.y += 1;
	}
	return ret_val;
}
DirectX::XMINT3 DefaultInput::GetTurn()
{
	assert(m_mouse);
	auto state = m_mouse->GetState();
	if (state.x != 0 || state.y != 0)
	{
		state.x += 1;
	}
	int turn_eq = 0;
	auto key_state = m_keyboard->GetState();
	if(key_state.Q)
	{
		turn_eq -= 1;
	}
	else if(key_state.E)
	{
		turn_eq += 1;
	}
	return {state.x,state.y,turn_eq };
}