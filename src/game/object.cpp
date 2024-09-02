#include"object.h"
#include"world.h"
#include"render_proxy.h"
#include "shading_model.h"
#include"renderer.h"
#include<fstream>
#include <string>
#include<algorithm>

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

template<int D>
struct SHs
{
	float shs[(D + 1) * (D + 1) * 3];
};
template<int D>
struct RichPoint
{
	float pos[3];
	float n[3];
	SHs<D> shs;
	float opacity;
	float scale[3];
	float rot[4];
};

void GaussianPoints::load(std::string path)
{
	m_vertex_position.clear();
	m_vertex_color.clear();
	m_cov3d.clear();

	std::ifstream fin(path, std::ios::in | std::ios::binary);
	fin.seekg(0, std::ios::end);
	int file_size = fin.tellg();
	fin.seekg(0, std::ios::beg);
	std::unique_ptr<char[]> buffer = std::make_unique<char[]>(file_size);
	fin.read(buffer.get(), file_size);
	assert(fin);
	fin.close();
	std::string ply_header_str(buffer.get());
	int header_size=ply_header_str.find("end_header\n") + 11;
	int point_size = sizeof(RichPoint<0>);
	assert((file_size - header_size) % point_size == 0);
	int point_num = min((file_size - header_size) / point_size, 5*1024*1024);
	m_vertex_position.reserve(point_num);
	m_vertex_color.reserve(point_num);
	m_cov3d.reserve(point_num);
	const RichPoint<0>* data = reinterpret_cast<RichPoint<0>*>(buffer.get() + header_size);

	std::vector<std::pair<float,size_t>> depth_vec;
	depth_vec.reserve(point_num);

	for (int i = 0; i < point_num; i++)
	{
		const RichPoint<0>& point = data[i];
		if (point.pos[2] < 0)
		{
			continue;
		}
		//pos
		m_vertex_position.emplace_back(DirectX::XMFLOAT3(point.pos[0], point.pos[1], point.pos[2]));
		depth_vec.emplace_back(std::pair(point.pos[2],depth_vec.size()));

		//color
		{
			const float C0 = 0.28209479177387814f;
			float r, g, b, a;
			r = point.shs.shs[0] * C0 + 0.5f;
			g = point.shs.shs[1] * C0 + 0.5f;
			b = point.shs.shs[2] * C0 + 0.5f;
			a = std::exp(point.opacity) / (1 + std::exp(point.opacity));
			r = max(min(r, 1.0f), 0.0f);
			g = max(min(g, 1.0f), 0.0f);
			b = max(min(b, 1.0f), 0.0f);
			a = max(min(a, 1.0f), 0.0f);
			m_vertex_color.emplace_back(DirectX::XMFLOAT4(r, g, b, a));
		}

		//cov3d
		{
			DirectX::XMFLOAT3 scale = { std::exp(point.scale[0]),std::exp(point.scale[1]) ,std::exp(point.scale[2]) };
			DirectX::XMVECTOR quate_rot = { point.rot[0],point.rot[1] ,point.rot[2] ,point.rot[3] };
			quate_rot = DirectX::XMVector4Normalize(quate_rot);
			float r = quate_rot.m128_f32[0];
			float x = quate_rot.m128_f32[1];
			float y = quate_rot.m128_f32[2];
			float z = quate_rot.m128_f32[3];
			DirectX::XMMATRIX transform_matrix{
				(1 - 2 * (y * y + z * z)) * scale.x,2 * (x * y + r * z) * scale.x,2 * (x * z - r * y) * scale.x,0,
				2 * (x * y - r * z) * scale.y,(1 - 2 * (x * x + z * z)) * scale.y,2 * (y * z + r * x) * scale.y,0,
				2 * (x * z + r * y) * scale.z,2 * (y * z - r * x) * scale.z,(1 - 2 * (x * x + y * y)) * scale.z,0,
				0,0,0,1
			};
			auto transform_matrix_t = DirectX::XMMatrixTranspose(transform_matrix);
			auto cov3d = DirectX::XMMatrixMultiply(transform_matrix_t, transform_matrix);

			m_cov3d.emplace_back(DirectX::XMFLOAT3X3(
				cov3d.r[0].m128_f32[0], cov3d.r[0].m128_f32[1], cov3d.r[0].m128_f32[2],
				cov3d.r[1].m128_f32[0], cov3d.r[1].m128_f32[1], cov3d.r[1].m128_f32[2],
				cov3d.r[2].m128_f32[0], cov3d.r[2].m128_f32[1], cov3d.r[2].m128_f32[2]
			));
		}
	}

	std::sort(depth_vec.begin(), depth_vec.end(), [](const std::pair<float, size_t>& a, const std::pair<float, size_t>& b)->bool {
		return a.first > b.first;
		});
	auto position_copy = m_vertex_position;
	auto color_copy = m_vertex_color;
	auto cov3d_copy = m_cov3d;
	for(int i=0;i< depth_vec.size();i++)
	{
		int point_index = depth_vec[i].second;
		m_vertex_position[i] = position_copy[point_index];
		m_vertex_color[i] = color_copy[point_index];
		m_cov3d[i] = cov3d_copy[point_index];
	}

	return;
}

GaussianPoints::GaussianPoints(std::shared_ptr<World> world , std::string asset ) :StaticMesh(world)
{
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
GaussianPoints::GaussianPoints(std::shared_ptr<World> world, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 scale, DirectX::XMFLOAT3 rotation, std::string asset):
	StaticMesh(world, position, scale, rotation)
{
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
	//TODO:build BVH
	int i = 0;
	for (i = 0; i+64 < m_vertex_position.size(); i+=64)
	{
		proxy->clusters_buffer.emplace_back(GaussianRenderProxy::GaussianCluster{ 64, i });
		for (int j = 0; j < 64; j++)
		{
			proxy->points_buffer.emplace_back(GaussianRenderProxy::GaussianPoint{ m_vertex_position[i+j], m_vertex_color[i + j],m_cov3d[i + j] });
		}
	}
	int remain_num = m_vertex_position.size() - i;
	if (remain_num != 0)
	{
		proxy->clusters_buffer.emplace_back(GaussianRenderProxy::GaussianCluster{ remain_num, i });
		for (; i < m_vertex_position.size(); i++)
		{
			proxy->points_buffer.emplace_back(GaussianRenderProxy::GaussianPoint{ m_vertex_position[i], m_vertex_color[i],m_cov3d[i] });
		}
	}

	return proxy;
}