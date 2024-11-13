#include "windowswindow.h"
#include <shellapi.h>
#include <chrono>
#include"device_input.h"

LRESULT CALLBACK Window::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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

    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEWHEEL:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_MOUSEHOVER:
        DirectX::Mouse::ProcessMessage(message, wParam, lParam);
        break;

    case WM_KEYUP:
    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_ESCAPE:
            PostQuitMessage(0);
            break;
        default:
            DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
            break;
        }
        break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
    
	return 0;
}

void Window::internel_MainLoop(HINSTANCE hInstance, int nCmdShow)
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
    m_window_close_callback();
    return;
}

void Window::Show(HINSTANCE hInstance, int nCmdShow)
{
    m_thread = std::thread(&Window::internel_MainLoop,this, hInstance, nCmdShow);
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(500ms);
    return;
}

void Window::RegisterWindowCloseCallback(std::function<void()> callback)
{
    m_window_close_callback = callback;
}

void Window::Close()
{
    //todo assert gamethread
    PostMessage(m_hwnd, WM_CLOSE, 0, 0);
    m_thread.join();
    return;
}

Window::Window(int H, int W) :m_thread()
{ 
    m_width = W; 
    m_height = H; 
    m_hwnd = 0; 

};

Window::~Window()
{
    if (m_thread.joinable())
    {
        m_thread.join();
    }
    return;
}