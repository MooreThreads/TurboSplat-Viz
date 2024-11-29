#include"object.h"
#include"world.h"
#include"render_proxy.h"
#include "shading_model.h"
#include"renderer.h"
#include<filesystem>
#include<fstream>
#include <string>
#include<algorithm>
#include"gaussian_loader.h"

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
			if (cached_render_proxy)
			{
				cur_world->GetScene()->AddRenderProxy(cached_render_proxy);
			}
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


void GaussianPoints::GenDefaultData()
{
	m_vertex_position.clear();
	m_vertex_color.clear();
	m_cov3d.clear();
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 32; j++)
		{
			m_vertex_position.emplace_back(DirectX::XMFLOAT3{ -1.0f+0.5f*i,-0.5f+0.5f*j,0.0f });
			m_vertex_color.emplace_back(DirectX::XMFLOAT4{ 1.0f, 0.0f, 0.0f, 0.5f });
			m_cov3d.emplace_back(DirectX::XMFLOAT3X3(0.01f, 0, 0, 0, 0.01f, 0, 0, 0, 0.01f));
		}
	}
}
void GaussianPoints::GenProfileData()
{
	m_vertex_position.clear();
	m_vertex_color.clear();
	m_cov3d.clear();
	for (int i = 0; i < 128; i++)
	{
		for (int j = 0; j < 128; j++)
		{
			m_vertex_position.emplace_back(DirectX::XMFLOAT3{ -1.0f + 0.02f * i,-0.5f + 0.02f * j,0.0f });
			m_vertex_color.emplace_back(DirectX::XMFLOAT4{ 1.0f, 0.0f, 0.0f, 0.5f });
			m_cov3d.emplace_back(DirectX::XMFLOAT3X3(0.0003f, 0, 0, 0, 0.0003f, 0, 0, 0, 0.0003f));
		}
	}
}



void GaussianPoints::load(std::string path)
{
	assert(std::filesystem::exists(path));
	bool cache_miss = true;
	if (std::filesystem::exists(path + ".cache"))
	{
		auto asset_update_time = std::filesystem::last_write_time(path);
		auto cache_create_time = std::filesystem::last_write_time(path + ".cache");
		if (cache_create_time > asset_update_time)
		{
			std::vector<uint8_t> bin_data;
			std::fstream stream(path + ".cache", std::ios_base::binary | std::ios_base::in);
			stream.seekg(0, std::ios::end);
			int filesize = stream.tellg();
			stream.seekg(0, std::ios::beg);
			bin_data.resize(filesize);
			stream.read((char*)bin_data.data(), filesize);
			Deserialization(bin_data);
			cache_miss = false;
		}
	}
	
	if(cache_miss)
	{
		GSLoader loader;
		static_cluster_size = 64;
		loader.Load(path,static_cluster_size);
		m_vertex_position = loader.position;
		m_vertex_color = loader.color;
		m_cov3d = loader.cov3d;
		m_cluster = loader.clusters;
		m_cluster_origin = loader.cluster_AABB_origin;
		m_cluster_extension = loader.cluster_AABB_extension;
		std::vector<uint8_t> save_data;
		Serialization(save_data);
		std::fstream stream(path + ".cache", std::ios_base::binary | std::ios_base::out);
		stream.write((const char*)save_data.data(), save_data.size());
		stream.close();
	}
	return;
}

void GaussianPoints::Serialization(std::vector<uint8_t>& output)
{
	output.clear();
	int points_num = m_vertex_position.size();
	assert(m_vertex_color.size() == points_num);
	assert(m_cov3d.size() == points_num);

	output.push_back(reinterpret_cast<uint8_t*>(&static_cluster_size)[0]);
	output.push_back(reinterpret_cast<uint8_t*>(&static_cluster_size)[1]);
	output.push_back(reinterpret_cast<uint8_t*>(&static_cluster_size)[2]);
	output.push_back(reinterpret_cast<uint8_t*>(&static_cluster_size)[3]);


	output.push_back(reinterpret_cast<uint8_t*>(&points_num)[0]);
	output.push_back(reinterpret_cast<uint8_t*>(&points_num)[1]);
	output.push_back(reinterpret_cast<uint8_t*>(&points_num)[2]);
	output.push_back(reinterpret_cast<uint8_t*>(&points_num)[3]);

	for (int i = 0; i < points_num; i++)
	{
		uint8_t* data_ptr = reinterpret_cast<uint8_t*>(m_vertex_position.data() + i);
		for (int j = 0; j < sizeof(DirectX::XMFLOAT3); j++)
		{
			output.push_back(data_ptr[j]);
		}
	}
	for (int i = 0; i < points_num; i++)
	{
		uint8_t* data_ptr = reinterpret_cast<uint8_t*>(m_vertex_color.data() + i);
		for (int j = 0; j < sizeof(DirectX::XMFLOAT4); j++)
		{
			output.push_back(data_ptr[j]);
		}
	}
	for (int i = 0; i < points_num; i++)
	{
		uint8_t* data_ptr = reinterpret_cast<uint8_t*>(m_cov3d.data() + i);
		for (int j = 0; j < sizeof(DirectX::XMFLOAT3X3); j++)
		{
			output.push_back(data_ptr[j]);
		}
	}

	int clusters_num = m_cluster.size();
	assert(m_cluster_origin.size() == clusters_num);
	assert(m_cluster_extension.size() == clusters_num);
	output.push_back(reinterpret_cast<uint8_t*>(&clusters_num)[0]);
	output.push_back(reinterpret_cast<uint8_t*>(&clusters_num)[1]);
	output.push_back(reinterpret_cast<uint8_t*>(&clusters_num)[2]);
	output.push_back(reinterpret_cast<uint8_t*>(&clusters_num)[3]);
	for (int i = 0; i < clusters_num; i++)
	{
		int points_num_in_cluster = m_cluster[i].size();
		output.push_back(reinterpret_cast<uint8_t*>(&points_num_in_cluster)[0]);
		output.push_back(reinterpret_cast<uint8_t*>(&points_num_in_cluster)[1]);
		output.push_back(reinterpret_cast<uint8_t*>(&points_num_in_cluster)[2]);
		output.push_back(reinterpret_cast<uint8_t*>(&points_num_in_cluster)[3]);
		for (int j = 0; j < points_num_in_cluster; j++)
		{
			int point_id = m_cluster[i][j];
			output.push_back(reinterpret_cast<uint8_t*>(&point_id)[0]);
			output.push_back(reinterpret_cast<uint8_t*>(&point_id)[1]);
			output.push_back(reinterpret_cast<uint8_t*>(&point_id)[2]);
			output.push_back(reinterpret_cast<uint8_t*>(&point_id)[3]);
		}
	}
	for (int i = 0; i < clusters_num; i++)
	{
		uint8_t* data_ptr = reinterpret_cast<uint8_t*>(m_cluster_origin.data() + i);
		for (int j = 0; j < sizeof(DirectX::XMFLOAT3); j++)
		{
			output.push_back(data_ptr[j]);
		}
	}
	for (int i = 0; i < clusters_num; i++)
	{
		uint8_t* data_ptr = reinterpret_cast<uint8_t*>(m_cluster_extension.data() + i);
		for (int j = 0; j < sizeof(DirectX::XMFLOAT3); j++)
		{
			output.push_back(data_ptr[j]);
		}
	}

}

void GaussianPoints::Deserialization(const std::vector<uint8_t>& data)
{
	m_vertex_position.clear();
	m_vertex_color.clear();
	m_cov3d.clear();
	m_cluster.clear();
	m_cluster_origin.clear();
	m_cluster_extension.clear();

	const uint8_t* data_ptr = data.data();
	static_cluster_size = reinterpret_cast<const int*>(data_ptr)[0];
	data_ptr += 4;
	int points_num = reinterpret_cast<const int*>(data_ptr)[0];
	data_ptr += 4;

	//position
	const DirectX::XMFLOAT3* position_data = reinterpret_cast<const DirectX::XMFLOAT3*>(data_ptr);
	for (int i = 0; i < points_num;i++)
	{
		m_vertex_position.push_back(position_data[i]);
	}
	data_ptr += sizeof(DirectX::XMFLOAT3) * points_num;
	//color
	const DirectX::XMFLOAT4* color_data = reinterpret_cast<const DirectX::XMFLOAT4*>(data_ptr);
	for (int i = 0; i < points_num; i++)
	{
		m_vertex_color.push_back(color_data[i]);
	}
	data_ptr += sizeof(DirectX::XMFLOAT4) * points_num;
	//cov
	const DirectX::XMFLOAT3X3* cov_data = reinterpret_cast<const DirectX::XMFLOAT3X3*>(data_ptr);
	for (int i = 0; i < points_num; i++)
	{
		m_cov3d.push_back(cov_data[i]);
	}
	data_ptr += sizeof(DirectX::XMFLOAT3X3) * points_num;

	int clusters_num = reinterpret_cast<const int*>(data_ptr)[0];
	data_ptr += 4;

	for (int i = 0; i < clusters_num; i++)
	{
		int points_num_in_cluster= reinterpret_cast<const int*>(data_ptr)[0];
		data_ptr += 4;
		const int* point_id_data= reinterpret_cast<const int*>(data_ptr);
		std::vector<int> point_id_list;
		point_id_list.reserve(points_num_in_cluster);
		for (int j = 0; j < points_num_in_cluster; j++)
		{
			point_id_list.push_back(point_id_data[j]);
		}
		data_ptr += sizeof(int) * points_num_in_cluster;
		m_cluster.push_back(point_id_list);
	}

	const DirectX::XMFLOAT3* origin_data = reinterpret_cast<const DirectX::XMFLOAT3*>(data_ptr);
	for (int i = 0; i < clusters_num; i++)
	{
		m_cluster_origin.push_back(origin_data[i]);
	}
	data_ptr += sizeof(DirectX::XMFLOAT3) * clusters_num;

	const DirectX::XMFLOAT3* extension_data = reinterpret_cast<const DirectX::XMFLOAT3*>(data_ptr);
	for (int i = 0; i < clusters_num; i++)
	{
		m_cluster_extension.push_back(extension_data[i]);
	}
	data_ptr += sizeof(DirectX::XMFLOAT3) * clusters_num;

	return;

}


GaussianPoints::GaussianPoints(std::shared_ptr<World> world , std::string asset, int sh_degree) :StaticMesh(world), static_cluster_size(0)
{
	assert(sh_degree == 0);
	m_shading_model_name = typeid(GaussianSplattingShadingModel).name();
	if (asset.size() == 0)
	{
		GenDefaultData();
	}
	else
	{
		file_path = asset;
		load(asset);
	}
}
GaussianPoints::GaussianPoints(std::shared_ptr<World> world, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 scale, DirectX::XMFLOAT3 rotation, std::string asset,int sh_degree):
	StaticMesh(world, position, scale, rotation), static_cluster_size(0)
{
	assert(sh_degree == 0);
	m_shading_model_name = typeid(GaussianSplattingShadingModel).name();
	if (asset.size() == 0)
	{
		GenDefaultData();
	}
	else
	{
		file_path = asset;
		load(asset);
	}
}

std::shared_ptr<RenderProxy> GaussianPoints::CreateRenderProxy()
{
	std::shared_ptr<GaussianRenderProxy> proxy = std::make_shared<GaussianRenderProxy>();
	proxy->shading_model = RendererModule::GetInst()->GetShadingModelObj(m_shading_model_name);
	proxy->world_transform = GetWorldTransform();
	proxy->b_render_resources_inited = false;
	proxy->device_static_resource.reset();
	
	
	for (int i = 0; i < m_cluster.size(); i++)
	{
		proxy->clusters_buffer.emplace_back(GaussianRenderProxy::GaussianCluster{ (int)m_cluster[i].size(), (int)proxy->points_buffer.size(),
			m_cluster_origin[i],m_cluster_extension[i]});

		for (int j = 0; j < m_cluster[i].size(); j++)
		{
			int point_id = m_cluster[i][j];
			proxy->points_buffer.emplace_back(GaussianRenderProxy::GaussianPoint{ m_vertex_position[point_id], m_vertex_color[point_id],m_cov3d[point_id] });
		}
	}

	return proxy;
}