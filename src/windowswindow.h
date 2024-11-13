#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif
#include<windows.h>
#include<thread>
#include<functional>

class Window
{
private:
	HWND m_hwnd;
	int m_height;
	int m_width;
	std::thread m_thread;
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void internel_MainLoop(HINSTANCE hInstance, int nCmdShow);
	std::function<void()> m_window_close_callback;
public:
	Window(int H, int W);
	void Show(HINSTANCE hInstance, int nCmdShow);
	HWND GetHwnd() const { return m_hwnd; }
	int GetHeight() const { return m_height; }
	int GetWidth() const { return m_width; }
	void Close();
	void RegisterWindowCloseCallback(std::function<void()> callback);
	~Window();
};