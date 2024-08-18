#include <windows.h>
#include "screen_splash.h"
#include "main_loop.h"
#include<memory>
#include "agility_sdk.h"

int WinMain(HINSTANCE hinst, HINSTANCE hprev,LPSTR cmdline,int show)
{
    ScreenSplash::GetInst()->Show(hinst,show);

    std::shared_ptr<MainLoop> main_loop=std::make_shared<MainLoop>();
    std::weak_ptr<MainLoop> weak_ref_mainloop = main_loop;
    ScreenSplash::GetInst()->RegisterWindowCloseCallback([&weak_ref_mainloop]() {
        auto main_loop = weak_ref_mainloop.lock();
        if(main_loop)
            main_loop->Stop();
        });
    main_loop->Init();
    main_loop->Loop();
    
    return WM_QUIT;
}
