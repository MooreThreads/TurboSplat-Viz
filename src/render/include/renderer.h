#include "render_dll_helper.h"
#include<map>
#include<memory>
#include"viewport_info.h"

typedef HWND RendererID;
class SceneRenderer;
class Scene;
class ShadingModel;

class RENDER_MODULE_API RendererModule
{
private:
	std::map < RendererID, std::shared_ptr< SceneRenderer> > m_renderer_map;
	std::map < std::string, std::shared_ptr< ShadingModel> > m_shading_model;
	RendererID CreateRendererID(WorldId world_id, HWND hwnd);
	std::shared_ptr<SceneRenderer> GetSceneRenderer(WorldId world_id, HWND hwnd);
	void RemoveSceneRenderer(WorldId world_id, HWND hwnd);
	void CreateSceneRenderer(WorldId world_id, ViewportInfo viewport_infos);
	void InitShaders();
public:
	RendererModule();
	static RendererModule* GetInst();
	void Render(std::shared_ptr<Scene> scene, ViewportInfo viewport_infos, int framecount);
	void RegisterViewportAndWolrd(WorldId world_id, ViewportInfo viewport_infos);
	void UnregisterViewportAndWolrd(WorldId world_id, ViewportInfo viewport_infos);
	std::shared_ptr< ShadingModel> GetShadingModelObj(const std::string& model_name);
};