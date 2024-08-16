#pragma once

#include<atomic>
#include<memory>
#include<string>
#include<DirectXMath.h>
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
	Object(std::shared_ptr<World> world);
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
	//p.s. Do not modify the cached data. Create a New one and copy the property
	std::shared_ptr<RenderProxy> cached_render_proxy;
	DirectX::XMMATRIX GetWorldTransform();
	virtual std::shared_ptr<RenderProxy> CreateRenderProxy();
public:
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 scale;
	DirectX::XMFLOAT3 rotation;
	std::string m_shading_model_name;
	SceneObject(std::shared_ptr<World> world);
	void DoRenderUpdate();
	
};

class StaticMesh :public SceneObject
{
public:
	StaticMesh(std::shared_ptr<World> world);
};