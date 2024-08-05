#include <windows.h>
#include "screen_splash.h"
#include "main_loop.h"

int WinMain(HINSTANCE hinst, HINSTANCE hprev,LPSTR cmdline,int show)
{
    ScreenSplash::GetInst()->Show(hinst,show);

    MainLoop main_loop;
    main_loop.init(ScreenSplash::GetInst()->GetHwnd(), ScreenSplash::GetInst()->GetHeight(), ScreenSplash::GetInst()->GetWidth());
    main_loop.loop();
    
    return WM_QUIT;
}
