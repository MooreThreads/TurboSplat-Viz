#include "screen_splash.h"
#include <shellapi.h>
#include <chrono>

LRESULT CALLBACK ScreenSplash::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
	case WM_PAINT:
		break;
    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

void ScreenSplash::internel_MainLoop(HINSTANCE hInstance, int nCmdShow)
{

    // Initialize the window class.
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = "GaussianSplattingViewer";
    RegisterClassEx(&windowClass);
    LPCSTR title("GaussianSplattingViewer");

    RECT windowRect = { 0, 0, m_width, m_height };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // Create the window and store a handle to it.
    m_hwnd = CreateWindow(
        windowClass.lpszClassName,
        title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,        // We have no parent window.
        nullptr,        // We aren't using menus.
        hInstance,
        nullptr);


    ShowWindow(m_hwnd, nCmdShow);

    MSG msg = {};
    while (msg.message != WM_QUIT )
    {
        // Process any messages in the queue.
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return;
}

void ScreenSplash::Show(HINSTANCE hInstance, int nCmdShow)
{
    m_thread = std::thread(&ScreenSplash::internel_MainLoop,this, hInstance, nCmdShow);
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(500ms);
    return;
}

void ScreenSplash::Close()
{
    //todo assert gamethread
    PostMessage(m_hwnd, WM_CLOSE, 0, 0);
    m_thread.join();
    return;
}

ScreenSplash* ScreenSplash::GetInst()
{
    static ScreenSplash inst(1080, 1920);
    return &inst;
}

ScreenSplash::ScreenSplash(int H, int W) :m_thread()
{ 
    m_width = W; 
    m_height = H; 
    m_hwnd = 0; 

};

ScreenSplash::~ScreenSplash()
{
    if (m_thread.joinable())
    {
        m_thread.join();
    }
    return;
}