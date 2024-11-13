#include"viewport_info.h"
#include"world.h"
#include "windowswindow.h"
#include<memory>
#include<vector>
#include<atomic>

class Viewport
{
private:
	ViewportInfo m_info;
	std::shared_ptr<World> m_world;
public:
	Viewport(ViewportInfo info, std::shared_ptr<World> World);
	ViewportInfo GetInfo() const { return m_info; }
	ViewportInfo& GetInfoRef()  { return m_info; }
	std::shared_ptr<World> GetWorld() { return m_world; }
	void Draw(int frame_count);
};

class MainLoop
{
private:
	std::vector<Viewport> m_viewports;
	ViewportInfo m_gameclient_viewport;
	std::shared_ptr<World> m_game_world;
	std::unique_ptr<Window> m_game_window;
	int m_frame_counter;
	std::atomic<bool> b_loop;
	
	void CreateGameWindow(HINSTANCE hinst);
	void RenderInit();
	void GameInit();
public:
	MainLoop();
	~MainLoop();
	
	void Init(HINSTANCE hinst);
	void Loop();
	void Stop();
};