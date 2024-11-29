#include"game_dll_helper.h"
#include"DirectXMath.h"
#include"Keyboard.h"
#include <windows.h>
#include"Mouse.h"
#include<memory>

class GAME_MODULE_API DefaultInput
{
public:
	std::unique_ptr<DirectX::Keyboard> m_keyboard;
	std::unique_ptr<DirectX::Mouse> m_mouse;

	DefaultInput();
	void Init(std::unique_ptr<DirectX::Keyboard>&& keybaord, std::unique_ptr<DirectX::Mouse>&& mouse, HWND window);
	static DefaultInput& GetInst();
	DirectX::XMFLOAT3 GetMovement();
	DirectX::XMINT3 GetTurn();
};