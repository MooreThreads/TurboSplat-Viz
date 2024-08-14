#pragma once

#include<atomic>
#include<memory>
#include<string>
typedef int ObjId;
class World;
class RenderProxy;
class Object
{
protected:
	ObjId obj_id;
	std::weak_ptr<World> world;
public:
	static std::atomic<ObjId> id_generator;
	static ObjId GenObjectId() { return id_generator.fetch_add(1); }
	Object();
	virtual ~Object() {};
	virtual void Erase();
	ObjId GetId() const { return obj_id; }
	virtual void tick() {}
	std::shared_ptr<World> GetWorld();
};

class SceneObject:public Object
{
protected:
	bool b_render_data_dirty;
	bool b_dynamic_render;
	std::string m_shading_model_name;
	std::shared_ptr<RenderProxy> cached_render_proxy;
	virtual std::shared_ptr<RenderProxy> CreateRenderProxy();
public:
	void DoRenderUpdate();
	SceneObject();
};