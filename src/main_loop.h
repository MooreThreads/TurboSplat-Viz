#include"scene_render.h"
#include<memory>

class MainLoop
{
private:
	int m_frame_counter;
	std::unique_ptr<SceneRender> m_scene_render;

	void render_init(HWND hwnd, int h, int w);
	void draw();
public:
	MainLoop();
	void init(HWND hwnd, int h, int w);
	void loop();
};