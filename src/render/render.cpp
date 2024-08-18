#include"renderer.h"
#include"scene_renderer.h"
#include<type_traits>
#include<assert.h>

RendererModule::RendererModule():m_renderer_map()
{
	InitShaders();

	return;
}

RendererID RendererModule::CreateRendererID(WorldId world_id, HWND hwnd)
{
	return hwnd;
}
std::shared_ptr<SceneRenderer> RendererModule::GetSceneRenderer(WorldId world_id, HWND hwnd)
{
	auto iter = m_renderer_map.find(CreateRendererID(world_id,hwnd));
	if (iter == m_renderer_map.end())
	{
		return nullptr;
	}
	return iter->second;
}
void RendererModule::RemoveSceneRenderer(WorldId world_id, HWND hwnd)
{
	m_renderer_map.erase(CreateRendererID(world_id, hwnd));
	return;
}
void RendererModule::CreateSceneRenderer(WorldId world_id, ViewportInfo viewport_infos)
{
	auto new_renderer=std::make_shared<SceneRenderer>(viewport_infos);
	
	m_renderer_map[CreateRendererID(world_id, viewport_infos.hwnd)] = new_renderer;
}
RendererModule* RendererModule::GetInst()
{
	static RendererModule inst;
	return &inst;
}
void RendererModule::Render(std::shared_ptr<Scene> scene, ViewportInfo viewport_infos, int framecount)
{
	std::shared_ptr<SceneRenderer> renderer = GetSceneRenderer(scene->GetId(), viewport_infos.hwnd);
	renderer->Update(*scene.get(), viewport_infos, framecount);
	renderer->Render(framecount);
}

void RendererModule::RegisterViewportAndWolrd(WorldId world_id, ViewportInfo viewport_infos)
{
	CreateSceneRenderer(world_id, viewport_infos);
}
void RendererModule::UnregisterViewportAndWolrd(WorldId world_id, ViewportInfo viewport_infos)
{
	RemoveSceneRenderer(world_id, viewport_infos.hwnd);
}

template<class T>
void RegisterShader(std::map < std::string, std::shared_ptr< ShadingModel> >& map)
{
	static_assert(std::is_base_of<ShadingModel, T>());
	auto new_shader = std::make_shared<T>();
	new_shader->Init();
	map[std::string(typeid(T).name())] = new_shader;
}

void RendererModule::InitShaders()
{
	m_shading_model.clear();
	RegisterShader<ScreenTriangleShadingModel>(m_shading_model);
	
}

std::shared_ptr< ShadingModel> RendererModule::GetShadingModelObj(std::string model_name)
{
	auto iter = m_shading_model.find(model_name);
	assert(iter != m_shading_model.end());
	return iter->second;
}