#include"main_loop.h"
#include "windowswindow.h"
#include"renderer.h"
#include<assert.h>
#include"device_input.h"

Viewport::Viewport(ViewportInfo info, std::shared_ptr<World> world):m_info(info),m_world(world)
{

}

void Viewport::Draw(int frame_count)
{
	
	RendererModule::GetInst()->Render(m_world->GetScene(), m_info, frame_count);
}

MainLoop::MainLoop():m_frame_counter(1)
{

}
void MainLoop::CreateGameWindow(HINSTANCE hinst)
{
	m_game_window = std::make_unique<Window>(1080, 1920);
	m_game_window->Show(hinst, true);
	m_game_window->RegisterWindowCloseCallback([this]() {
			this->Stop();
		});
	ViewportInfo info;
	info.hwnd = m_game_window->GetHwnd();
	info.height = m_game_window->GetHeight();
	info.width = m_game_window->GetWidth();
	m_viewports.emplace_back(info,m_game_world);
}
void MainLoop::RenderInit()
{
	RendererModule::GetInst()->RegisterViewportAndWolrd(m_game_world->GetId(), m_viewports[0].GetInfo());
	return;
}

void MainLoop::GameInit()
{
	m_game_world = std::make_shared<DefaultWorld>();
	m_game_world->Init();
	m_game_world->RegisterDefaultWorld();
	return;
}


void MainLoop::Init(HINSTANCE hinst)
{
	GameInit();

	CreateGameWindow(hinst);

	DefaultInput::GetInst().Init(std::make_unique<DirectX::Keyboard>(), std::make_unique<DirectX::Mouse>(), m_viewports[0].GetInfo().hwnd);
	
	RenderInit();


	return;
}

void MainLoop::Loop()
{

	b_loop = true;
	while (b_loop)
	{
		for (auto& viewport : m_viewports)
		{
			//gamethread tick
			auto cur_world=viewport.GetWorld();
			cur_world->Tick(m_frame_counter, 1);
			cur_world->DoRenderUpdates(viewport.GetInfoRef());
			viewport.Draw(m_frame_counter);
		}
		m_frame_counter++;
	}
	return;
}

void MainLoop::Stop()
{
	b_loop = false;
}

MainLoop::~MainLoop()
{
	for (auto& viewport : m_viewports)
	{
		RendererModule::GetInst()->UnregisterViewportAndWolrd(m_game_world->GetId(), m_viewports[0].GetInfo());
	}
	return;
}