#include "world.h"
#include<vector>
#include<assert.h>


std::weak_ptr<World> World::default_world_ptr;
std::atomic<WorldId> World::worldid_generator(0);

World::World():m_objs()
{
	id = worldid_generator.fetch_add(1);
	m_scene = std::make_shared<Scene>(id);

}

void World::AddObjectInternel(std::shared_ptr<Object> obj)
{
	m_objs[obj->GetId()]= obj;
}

void World::RemoveObjectInternel(ObjId id)
{
	m_objs.erase(id);
}

void World::AddObject(std::shared_ptr<Object> obj)
{
	m_new_objs[obj->GetId()] = obj;
}

void World::RemoveObject(ObjId id)
{
	m_erase_objs.push_back(id);
}

void World::Tick(int cur_frame, int duration)
{
	//obj tick
	for (auto pairs : m_objs)
	{
		ObjId id = pairs.first;
		std::shared_ptr<Object> obj = pairs.second;
		if (obj != nullptr)
		{
			obj->tick();
		}
	}

	//modify m_objs
	for (auto pairs : m_new_objs)
	{
		m_objs.insert(pairs);
	}
	for (auto id : m_erase_objs)
	{
		RemoveObjectInternel(id);
	}
	m_erase_objs.clear();
	m_new_objs.clear();
}

void World::DoRenderUpdates()
{
	//reset scene
	m_scene->ClearTemporal();

	//update objs
	for (auto pairs : m_objs)
	{
		std::shared_ptr<Object> obj = pairs.second;
		if (obj)
		{
			SceneObject* scene_obj = dynamic_cast<SceneObject*>(obj.get());
			if (scene_obj)
			{
				scene_obj->DoRenderUpdate();
			}
		}
	}
}

std::shared_ptr<World> World::GetDefaultWorld()
{
	return default_world_ptr.lock();
}

void World::RegisterDefaultWorld()
{
	default_world_ptr = shared_from_this();
}

TestWorld::TestWorld():World()
{

}

void TestWorld::Init()
{
	//create objs
	// 
	//AddObject()
	AddObjectInternel(std::make_shared<SceneObject>());
}