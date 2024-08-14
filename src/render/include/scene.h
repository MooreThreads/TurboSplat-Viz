#pragma once
#include<vector>
#include<memory>
#include"render_dll_helper.h"
class RenderProxy;
typedef int WorldId;
class RENDER_MODULE_API Scene
{
protected:
	const WorldId m_worldId;
	std::vector<std::shared_ptr<RenderProxy>> m_render_proxy;
	std::vector<std::shared_ptr<RenderProxy>> m_temproal_render_proxy;
public:
	Scene(WorldId world_id);
	Scene(const Scene& other);
	void operator=(const Scene& other);
	void AddRenderProxy(std::shared_ptr<RenderProxy> proxy);
	void RemoveRenderProxy(std::shared_ptr<RenderProxy> proxy);
	void ClearTemporal();
	void AddTemporalProxy(std::shared_ptr<RenderProxy> proxy);
	WorldId GetId() const { return m_worldId; }
	const std::vector<std::shared_ptr<RenderProxy>>& GetRenderProxy() const { return m_render_proxy; }
	const std::vector<std::shared_ptr<RenderProxy>>& GetTemproalRenderProxy() const { return m_temproal_render_proxy; }

};