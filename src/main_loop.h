#include"render_threads_pool.h"

class MainLoop
{
private:
	int m_frame_counter;
	RenderThreadsPool m_render_thread_pool;

	void render_init();
	void draw();
public:
	MainLoop();
	void init();
	void loop();
};