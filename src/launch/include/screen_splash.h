#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif
#include<windows.h>
#include<thread>

#if _WIN32
	#ifdef DLLIMPORT
		#define LIB_API __declspec(dllimport)
	#else
		#define LIB_API __declspec(dllexport)
	#endif
#else
#define LIB_API extern
#endif


class LIB_API ScreenSplash
{
private:
	HWND m_hwnd;
	int m_height;
	int m_width;
	std::thread m_thread;
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void internel_MainLoop(HINSTANCE hInstance, int nCmdShow);
public:
	ScreenSplash(int H, int W);
	void Show(HINSTANCE hInstance, int nCmdShow);
	HWND GetHwnd() { return m_hwnd; };
	static ScreenSplash* GetInst();
	void Close();
	~ScreenSplash();
};