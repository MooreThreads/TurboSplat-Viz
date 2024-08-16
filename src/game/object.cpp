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

SceneObject::SceneObject(std::shared_ptr<World> world):Object(world),b_dynamic_render(false),b_render_data_dirty(true), position(0,0,0),scale(0,0,0),rotation(0,0,0)
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
	DirectX::XMMATRIX ret(
		scale.x,	0,			0,			position.x,
		0,			scale.y,	0,			position.y,
		0,			0,			scale.z,	position.z,
		0,			0,			0,			1
	);
	return ret;
}

std::shared_ptr<RenderProxy> SceneObject::CreateRenderProxy()
{
	std::shared_ptr<ScreenTriangleRenderProxy> proxy = std::make_shared<ScreenTriangleRenderProxy>();
	proxy->vertex.emplace_back(ScreenTriangleRenderProxy::Vertex{ { 0.0f, 0.25f , 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } });
	proxy->vertex.emplace_back(ScreenTriangleRenderProxy::Vertex{ { 0.25f, -0.25f , 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } });
	proxy->vertex.emplace_back(ScreenTriangleRenderProxy::Vertex{ { -0.25f, -0.25f , 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } });
	proxy->shading_model = RendererModule::GetInst()->GetShadingModelObj(m_shading_model_name);
	proxy->b_render_resources_inited = false;
	proxy->device_static_resource.reset();
	proxy->world_transform = GetWorldTransform();
	return proxy;
}