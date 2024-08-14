#include"scene.h"
#include"render_proxy.h"

Scene::Scene(WorldId world_id):m_worldId(world_id),m_render_proxy(),m_temproal_render_proxy()
{

}
Scene::Scene(const Scene& other):m_worldId(other.m_worldId)
{
	m_render_proxy = other.m_render_proxy;
	m_temproal_render_proxy = other.m_temproal_render_proxy;
}

void Scene::operator=(const Scene& other)
{
	m_render_proxy = other.m_render_proxy;
	m_temproal_render_proxy = other.m_temproal_render_proxy;
}
void Scene::AddRenderProxy(std::shared_ptr<RenderProxy> proxy)
{
	m_render_proxy.push_back(proxy);
}
void Scene::RemoveRenderProxy(std::shared_ptr<RenderProxy> proxy)
{
	assert(false);
	//todo
}
void Scene::ClearTemporal()
{
	m_temproal_render_proxy.clear();
}
void Scene::AddTemporalProxy(std::shared_ptr<RenderProxy> proxy)
{
	m_temproal_render_proxy.push_back(proxy);
}