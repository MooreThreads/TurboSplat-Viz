#include "world.h"
#include<vector>
#include<assert.h>
#include<iostream>
#include"inicpp/inicpp.hpp"


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
	if (m_objs.find(id) != m_objs.end())
	{
		m_objs.erase(id);
	}
	else if (m_cameras.find(id) != m_cameras.end())
	{
		DetachCamera(id);
	}
}

void World::AttachCamera(std::shared_ptr<Camera> camera)
{
	m_cameras[camera->GetId()] = camera;
}

void World::DetachCamera(ObjId id)
{
	m_cameras.erase(id);
}

void World::AddObject(std::shared_ptr<Object> obj)
{
	
	std::shared_ptr<Camera> camera = std::dynamic_pointer_cast<Camera>(obj);
	if (camera)
	{
		AttachCamera(camera);
	}
	else
	{
		m_new_objs[obj->GetId()] = obj;
	}
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
	for (auto id_and_camera : m_cameras)
	{
		std::shared_ptr<Camera> camera = id_and_camera.second;
		camera->tick();
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

void World::DoRenderUpdates(ViewportInfo& viewport_info)
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

	viewport_info.views.clear();
	for (auto id_and_camera : m_cameras)
	{
		std::shared_ptr<Camera> camera = id_and_camera.second;
		ViewInfo view;
		view.m_viewport= CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(viewport_info.width), static_cast<float>(viewport_info.height));
		view.m_scissor_rect = CD3DX12_RECT(0, 0, static_cast<LONG>(viewport_info.width), static_cast<LONG>(viewport_info.height));
		view.view_matrix = camera->GetViewMatrix();
		view.project_matrix = camera->GetProjectMatrix(viewport_info.width, viewport_info.height,true);
		view.focal = camera->GetFocal(viewport_info.width, viewport_info.height);
		view.render_target_view = CD3DX12_CPU_DESCRIPTOR_HANDLE();

		//frustum_plane
		auto view_proj_matrix = DirectX::XMMatrixMultiply(view.view_matrix, view.project_matrix);
		//left plane
		view.frustum_plane[0].x = view_proj_matrix.r[0].m128_f32[3] + view_proj_matrix.r[0].m128_f32[0];
		view.frustum_plane[0].y = view_proj_matrix.r[1].m128_f32[3] + view_proj_matrix.r[1].m128_f32[0];
		view.frustum_plane[0].z = view_proj_matrix.r[2].m128_f32[3] + view_proj_matrix.r[2].m128_f32[0];
		view.frustum_plane[0].w = view_proj_matrix.r[3].m128_f32[3] + view_proj_matrix.r[3].m128_f32[0];
		//right plane
		view.frustum_plane[1].x = view_proj_matrix.r[0].m128_f32[3] - view_proj_matrix.r[0].m128_f32[0];
		view.frustum_plane[1].y = view_proj_matrix.r[1].m128_f32[3] - view_proj_matrix.r[1].m128_f32[0];
		view.frustum_plane[1].z = view_proj_matrix.r[2].m128_f32[3] - view_proj_matrix.r[2].m128_f32[0];
		view.frustum_plane[1].w = view_proj_matrix.r[3].m128_f32[3] - view_proj_matrix.r[3].m128_f32[0];
		//bottom plane
		view.frustum_plane[2].x = view_proj_matrix.r[0].m128_f32[3] + view_proj_matrix.r[0].m128_f32[1];
		view.frustum_plane[2].y = view_proj_matrix.r[1].m128_f32[3] + view_proj_matrix.r[1].m128_f32[1];
		view.frustum_plane[2].z = view_proj_matrix.r[2].m128_f32[3] + view_proj_matrix.r[2].m128_f32[1];
		view.frustum_plane[2].w = view_proj_matrix.r[3].m128_f32[3] + view_proj_matrix.r[3].m128_f32[1];
		//right plane
		view.frustum_plane[3].x = view_proj_matrix.r[0].m128_f32[3] - view_proj_matrix.r[0].m128_f32[1];
		view.frustum_plane[3].y = view_proj_matrix.r[1].m128_f32[3] - view_proj_matrix.r[1].m128_f32[1];
		view.frustum_plane[3].z = view_proj_matrix.r[2].m128_f32[3] - view_proj_matrix.r[2].m128_f32[1];
		view.frustum_plane[3].w = view_proj_matrix.r[3].m128_f32[3] - view_proj_matrix.r[3].m128_f32[1];
		//near plane
		view.frustum_plane[4].x = view_proj_matrix.r[0].m128_f32[2];
		view.frustum_plane[4].y = view_proj_matrix.r[1].m128_f32[2];
		view.frustum_plane[4].z = view_proj_matrix.r[2].m128_f32[2];
		view.frustum_plane[4].w = view_proj_matrix.r[3].m128_f32[2];
		//far plane
		view.frustum_plane[5].x = view_proj_matrix.r[0].m128_f32[3] - view_proj_matrix.r[0].m128_f32[2];
		view.frustum_plane[5].y = view_proj_matrix.r[1].m128_f32[3] - view_proj_matrix.r[1].m128_f32[2];
		view.frustum_plane[5].z = view_proj_matrix.r[2].m128_f32[3] - view_proj_matrix.r[2].m128_f32[2];
		view.frustum_plane[5].w = view_proj_matrix.r[3].m128_f32[3] - view_proj_matrix.r[3].m128_f32[2];

		viewport_info.views.push_back(view);
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

DefaultWorld::DefaultWorld():World()
{

}

void DefaultWorld::Init()
{
	//create objs

	inicpp::IniManager config("../asset/default_scene.ini");
	auto obj_name_list = config.getSectionsList();
	for (auto obj_name : obj_name_list)
	{
		auto file_name = config[obj_name].toString("file");
		auto type = config[obj_name].toString("type");
		if (type == "GS")
		{
			int sh_degree= config[obj_name].toInt("sh");
			if (sh_degree != 0)
			{
				std::cout << "default_scene - " << obj_name << ": Currently, only scenes with sh_degree set to 0 are supported";
				continue;
			}
			//float pos_x = config[obj_name].toDouble("position_x");
			//float pos_y = config[obj_name].toDouble("position_y");
			//float pos_z = config[obj_name].toDouble("position_z");
			auto obj = std::make_shared<GaussianPoints>(shared_from_this(), DirectX::XMFLOAT3{ 0,0,0 }, DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f }, DirectX::XMFLOAT3{ 0.0f,0.0f,0.0f }, "../asset/"+file_name);
			obj->Init();
		}
		else
		{
			std::cout << "default_scene - " << obj_name << ": type error";
			continue;
		}
		

	}




	auto camera = std::make_shared<Camera>(shared_from_this(), 90,0.1,100, DirectX::XMFLOAT3{ 0,0,0 }, DirectX::XMFLOAT3{ -90,0,0 });
	camera->Init();
}