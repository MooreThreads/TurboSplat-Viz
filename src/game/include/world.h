#pragma once
#include"object.h"
#include"scene.h"
#include<map>
#include<atomic>
#include<vector>
#include<memory>
#include "game_dll_helper.h"
#include "camera.h"
#include "viewport_info.h"

class GAME_MODULE_API World:public std::enable_shared_from_this<World>
{
protected:
	WorldId id;
	std::map<ObjId, std::shared_ptr<Object>> m_objs;
	std::map<ObjId, std::shared_ptr<Camera>> m_cameras;
	std::vector<ObjId> m_erase_objs;
	std::map<ObjId, std::shared_ptr<Object>> m_new_objs;
	std::shared_ptr<Scene> m_scene;
	void AddObjectInternel(std::shared_ptr<Object>);
	void RemoveObjectInternel(ObjId id);
	void AttachCamera(std::shared_ptr<Camera>);
	void DetachCamera(ObjId id);

	static std::weak_ptr<World> default_world_ptr;
	static std::atomic<WorldId> worldid_generator;
public:
	World();
	std::shared_ptr<Scene> GetScene() { return m_scene; }
	WorldId GetId() const { return id; }
	virtual void Init() {};
	void AddObject(std::shared_ptr<Object>);
	void RemoveObject(ObjId id);
	void Tick(int cur_frame,int duration);
	void DoRenderUpdates(ViewportInfo& viewport_info);
	
	void RegisterDefaultWorld();
	static std::shared_ptr<World> GetDefaultWorld();

	
};

class GAME_MODULE_API DefaultWorld :public World
{
protected:
public:
	DefaultWorld();
	virtual void Init();
};