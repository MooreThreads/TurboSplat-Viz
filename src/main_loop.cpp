#include"main_loop.h"
#include"render_threads_pool.h"
#include "screen_splash.h"
#include"renderer.h"
#include<assert.h>

Viewport::Viewport(ViewportInfo info, std::shared_ptr<World> world):m_info(info),m_world(world)
{

}

void Viewport::Draw(int frame_count)
{
	
	RendererModule::GetInst()->Render(m_world->GetScene(), m_info, frame_count);
}

MainLoop::MainLoop():m_frame_counter(0)
{

}
void MainLoop::CreateGameClientViewport()
{
	ViewportInfo info;
	info.hwnd = ScreenSplash::GetInst()->GetHwnd();
	info.height = ScreenSplash::GetInst()->GetHeight();
	info.width = ScreenSplash::GetInst()->GetWidth();
	m_viewports.emplace_back(info,m_game_world);
}
void MainLoop::RenderInit()
{
	RendererModule::GetInst()->RegisterViewportAndWolrd(m_game_world->GetId(), m_viewports[0].GetInfo());
	return;
}

void MainLoop::GameInit()
{
	m_game_world = std::make_shared<TestWorld>();
	m_game_world->Init();
	m_game_world->RegisterDefaultWorld();
	return;
}


void MainLoop::Init()
{
	GameInit();

	CreateGameClientViewport();
	
	RenderInit();

	return;
}

void MainLoop::Loop()
{
	while (true)
	{
		//if gamethread exit or renderthread crash
		//exit

		//gamethread tick
		m_game_world->Tick(m_frame_counter,1);
		m_game_world->DoRenderUpdates();

		//viewport draw
		for (auto& viewport : m_viewports)
		{
			viewport.Draw(m_frame_counter);
		}


		m_frame_counter++;

		//check window close

	}
	return;
}