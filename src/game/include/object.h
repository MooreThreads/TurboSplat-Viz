#pragma once

#include<atomic>
#include<memory>
#include<vector>
#include<string>
#include<DirectXMath.h>
typedef int ObjId;
class World;
class RenderProxy;
class Object:public std::enable_shared_from_this<Object>
{
protected:
	ObjId obj_id;
	std::weak_ptr<World> world;
	static std::atomic<ObjId> id_generator;
	static ObjId GenObjectId() { return id_generator.fetch_add(1); }
public:

	Object(std::shared_ptr<World> world);
	virtual ~Object() {};
	virtual void Init();
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
	virtual ~SceneObject() {};
	
};

class StaticMesh :public SceneObject
{
protected:
	void GenDefaultData();
public:
	std::vector<DirectX::XMFLOAT3> m_vertex_position;
	std::vector<DirectX::XMFLOAT4> m_vertex_color;
	StaticMesh(std::shared_ptr<World> world);
	StaticMesh(std::shared_ptr<World> world, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 scale, DirectX::XMFLOAT3 rotation);
	virtual std::shared_ptr<RenderProxy> CreateRenderProxy();
	virtual ~StaticMesh() {};
};

class AlphaStaticMesh :public StaticMesh
{
public:
	AlphaStaticMesh(std::shared_ptr<World> world);
	AlphaStaticMesh(std::shared_ptr<World> world, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 scale, DirectX::XMFLOAT3 rotation);
	virtual ~AlphaStaticMesh() {};
};

class GaussianPoints :public StaticMesh
{
protected:
	void GenDefaultData();
public:
	std::vector<DirectX::XMFLOAT3X3> m_cov3d;
	GaussianPoints(std::shared_ptr<World> world);
	GaussianPoints(std::shared_ptr<World> world, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 scale, DirectX::XMFLOAT3 rotation);
	virtual std::shared_ptr<RenderProxy> CreateRenderProxy();
	virtual ~GaussianPoints() {};
};