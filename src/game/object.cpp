#include"object.h"
#include"world.h"
#include"render_proxy.h"
#include "shading_model.h"
#include"renderer.h"

std::atomic<ObjId> Object::id_generator(0);

Object::Object()
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

SceneObject::SceneObject():Object(),b_dynamic_render(false),b_render_data_dirty(true)
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
		cur_world->GetScene()->AddRenderProxy(cached_render_proxy);
		
		if (b_dynamic_render)//todo
		{
			b_render_data_dirty = true;
		}

	}
}

std::shared_ptr<RenderProxy> SceneObject::CreateRenderProxy()
{
	std::shared_ptr<RenderProxy> proxy = std::make_shared<ScreenTriangleRenderProxy>();
	proxy->shading_model = RendererModule::GetInst()->GetShadingModelObj(m_shading_model_name);
	return proxy;
}