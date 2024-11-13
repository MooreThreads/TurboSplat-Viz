#include <windows.h>
#include "main_loop.h"
#include<memory>
#include "agility_sdk.h"

int WinMain(HINSTANCE hinst, HINSTANCE hprev,LPSTR cmdline,int show)
{
    std::shared_ptr<MainLoop> main_loop=std::make_shared<MainLoop>();
    main_loop->Init(hinst);
    main_loop->Loop();
    
    return WM_QUIT;
}
