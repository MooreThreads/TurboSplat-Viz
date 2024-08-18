#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif
#include<windows.h>
#include<thread>

#if _WIN32
	#ifdef DLLIMPORT
		#define LAUNCH_MODULE_API __declspec(dllimport)
	#else
		#define LAUNCH_MODULE_API __declspec(dllexport)
	#endif
#else
#define LAUNCH_MODULE_API extern
#endif
#include<functional>

class LAUNCH_MODULE_API ScreenSplash
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
	ScreenSplash(int H, int W);
	void Show(HINSTANCE hInstance, int nCmdShow);
	HWND GetHwnd() const { return m_hwnd; }
	int GetHeight() const { return m_height; }
	int GetWidth() const { return m_width; }
	static ScreenSplash* GetInst();
	void Close();
	void RegisterWindowCloseCallback(std::function<void()> callback);
	~ScreenSplash();
};