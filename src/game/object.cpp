#include"object.h"
#include"world.h"
#include"render_proxy.h"
#include "shading_model.h"
#include"renderer.h"

std::atomic<ObjId> Object::id_generator(0);

Object::Object(std::shared_ptr<World> world):world(world)
{
	obj_id = Object::GenObjectId();
}
void Object::Init()
{
	auto cur_world = world.lock();
	if (cur_world)
	{
		cur_world->AddObject(shared_from_this());
	}
	else 
	{
		assert(false);
	}
}
void Object::Erase()
{
	auto belong_to = world.lock();
	if (belong_to)
	{
		belong_to->RemoveObject(obj_id);
	}
}
std::shared_ptr<World> Object::GetWorld()
{
	return world.lock();
}

SceneObject::SceneObject(std::shared_ptr<World> world):Object(world),b_dynamic_render(false),b_render_data_dirty(true), position(0,0,0),scale(1,1,1),rotation(0,0,0)
{
	m_shading_model_name=typeid(ScreenTriangleShadingModel).name();
}

void SceneObject::DoRenderUpdate()
{
	std::shared_ptr<World> cur_world = GetWorld();
	if (cur_world)
	{

		if (b_render_data_dirty)
		{
			cached_render_proxy=CreateRenderProxy();
			cur_world->GetScene()->AddRenderProxy(cached_render_proxy);
			b_render_data_dirty = false;
		}
		
		if (b_dynamic_render)//todo
		{
			b_render_data_dirty = true;
		}

	}
}
DirectX::XMMATRIX SceneObject::GetWorldTransform()
{
	DirectX::XMMATRIX rot=DirectX::XMMatrixRotationRollPitchYaw(rotation.x/180*DirectX::XM_PI, rotation.y / 180 * DirectX::XM_PI, rotation.z / 180 * DirectX::XM_PI);//LH Default
	DirectX::XMMATRIX ret(
		scale.x,	0,			0,			0,
		0,			scale.y,	0,			0,
		0,			0,			scale.z,	0,
		position.x, position.y, position.z,	1
	);

	ret = DirectX::XMMatrixMultiply(rot, ret);

	return ret;
}

std::shared_ptr<RenderProxy> SceneObject::CreateRenderProxy()
{
	std::shared_ptr<TriangleRenderProxy> proxy = std::make_shared<TriangleRenderProxy>();
	proxy->vertex.emplace_back(TriangleRenderProxy::Vertex{ { 0.0f, 0.25f , 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } });
	proxy->vertex.emplace_back(TriangleRenderProxy::Vertex{ { 0.25f, -0.25f , 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } });
	proxy->vertex.emplace_back(TriangleRenderProxy::Vertex{ { -0.25f, -0.25f , 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } });
	proxy->shading_model = RendererModule::GetInst()->GetShadingModelObj(m_shading_model_name);
	proxy->b_render_resources_inited = false;
	proxy->device_static_resource.reset();
	proxy->world_transform = GetWorldTransform();
	return proxy;
}

void StaticMesh::GenDefaultData()
{
	m_vertex_position.emplace_back(DirectX::XMFLOAT3{ 0.0f,0.25f,0.0f });
	m_vertex_color.emplace_back(DirectX::XMFLOAT4{ 1.0f, 0.0f, 0.0f, 0.5f });
	m_vertex_position.emplace_back(DirectX::XMFLOAT3{ 0.25f, -0.25f , 0.0f });
	m_vertex_color.emplace_back(DirectX::XMFLOAT4{ 0.0f, 1.0f, 0.0f, 0.5f });
	m_vertex_position.emplace_back(DirectX::XMFLOAT3{ -0.25f, -0.25f , 0.0f });
	m_vertex_color.emplace_back(DirectX::XMFLOAT4{ 0.0f, 0.0f, 1.0f, 0.5f });

	m_vertex_position.emplace_back(DirectX::XMFLOAT3{ 0.2f,0.25f,-0.2f });
	m_vertex_color.emplace_back(DirectX::XMFLOAT4{ 1.0f, 0.0f, 0.0f, 0.5f });
	m_vertex_position.emplace_back(DirectX::XMFLOAT3{ 0.45f, -0.25f , -0.2f });
	m_vertex_color.emplace_back(DirectX::XMFLOAT4{ 0.0f, 1.0f, 0.0f, 0.5f });
	m_vertex_position.emplace_back(DirectX::XMFLOAT3{ -0.05f, -0.25f , -0.2f });
	m_vertex_color.emplace_back(DirectX::XMFLOAT4{ 0.0f, 0.0f, 1.0f, 0.5f });
}

StaticMesh::StaticMesh(std::shared_ptr<World> world):SceneObject(world)
{
	m_shading_model_name = typeid(BasicMeshShadingModel).name();
	GenDefaultData();
}
StaticMesh::StaticMesh(std::shared_ptr<World> world, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 scale, DirectX::XMFLOAT3 rotation) :
	SceneObject(world)
{
	this->position = position;
	this->scale = scale;
	this->rotation = rotation;
	m_shading_model_name = typeid(BasicMeshShadingModel).name();
	GenDefaultData();
}

std::shared_ptr<RenderProxy> StaticMesh::CreateRenderProxy()
{
	std::shared_ptr<TriangleRenderProxy> proxy = std::make_shared<TriangleRenderProxy>();
	for (int i = 0; i < m_vertex_position.size(); i++)
	{
		proxy->vertex.emplace_back(TriangleRenderProxy::Vertex{ m_vertex_position[i], m_vertex_color[i]});
	}
	proxy->shading_model = RendererModule::GetInst()->GetShadingModelObj(m_shading_model_name);
	proxy->b_render_resources_inited = false;
	proxy->device_static_resource.reset();
	proxy->world_transform = GetWorldTransform();
	return proxy;
}

AlphaStaticMesh::AlphaStaticMesh(std::shared_ptr<World> world):StaticMesh(world)
{
	m_shading_model_name = typeid(AlphaMeshShadingModel).name();
}

AlphaStaticMesh::AlphaStaticMesh(std::shared_ptr<World> world, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 scale, DirectX::XMFLOAT3 rotation) :StaticMesh(world, position, scale, rotation)
{
	m_shading_model_name = typeid(AlphaMeshShadingModel).name();
}