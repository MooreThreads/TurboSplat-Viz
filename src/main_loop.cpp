#include"main_loop.h"
#include"render_threads_pool.h"
#include<assert.h>

MainLoop::MainLoop():m_frame_counter(0),m_scene_render()
{

}
void MainLoop::render_init(HWND hwnd, int h, int w)
{
	RenderThreadsPool::GetInst()->Init(1,hwnd,h,w);
	m_scene_render = std::make_unique<SceneRender>();
	return;
}
void MainLoop::draw()
{
	return;
}

void MainLoop::init(HWND hwnd, int h, int w)
{
	//todo game threads init
	render_init(hwnd, h, w);

	return;
}

void MainLoop::loop()
{
	assert(m_scene_render != nullptr);
	while (true)
	{
		//if gamethread exit or renderthread crash
		//exit

		//gamethread tick

		//scene render draw
		m_scene_render->render(m_frame_counter);


		m_frame_counter++;
	}
	return;
}