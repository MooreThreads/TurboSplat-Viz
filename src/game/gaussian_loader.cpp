#include"gaussian_loader.h"
#include<fstream>
#include<limits>
#include <algorithm>

typedef std::numeric_limits< float> flt;
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

void GSLoader::Load(std::string path, int static_cluster_size)
{
	assert(static_cluster_size != 0);//do not support dynamic size yet.
	position.clear();
	color.clear();
	cov3d.clear();
	AABB_extension.clear();

	std::ifstream fin(path, std::ios::in | std::ios::binary);
	fin.seekg(0, std::ios::end);
	int file_size = fin.tellg();
	fin.seekg(0, std::ios::beg);
	std::unique_ptr<char[]> buffer = std::make_unique<char[]>(file_size);
	fin.read(buffer.get(), file_size);
	assert(fin);
	fin.close();
	std::string ply_header_str(buffer.get());
	int header_size = ply_header_str.find("end_header\n") + 11;
	int point_size = sizeof(RichPoint<0>);
	assert((file_size - header_size) % point_size == 0);
	int point_num = (file_size - header_size) / point_size;
	assert(point_num <= 50 * 1024 * 1024);
	position.reserve(point_num);
	color.reserve(point_num);
	cov3d.reserve(point_num);
	const RichPoint<0>* data = reinterpret_cast<RichPoint<0>*>(buffer.get() + header_size);

	std::vector<std::pair<float, size_t>> depth_vec;
	depth_vec.reserve(point_num);

	for (int i = 0; i < point_num; i++)
	{
		const RichPoint<0>& point = data[i];

		//alhpa
		float alpha = std::exp(point.opacity) / (1 + std::exp(point.opacity));
		if (alpha <= 1.0f / 255)
		{
			continue;
		}
		if (std::isnan(point.pos[0])|| std::isnan(point.rot[0]))
		{
			continue;
		}

		//color
		{
			const float C0 = 0.28209479177387814f;
			float r, g, b;
			r = point.shs.shs[0] * C0 + 0.5f;
			g = point.shs.shs[1] * C0 + 0.5f;
			b = point.shs.shs[2] * C0 + 0.5f;
			r = std::max(std::min(r, 1.0f), 0.0f);
			g = std::max(std::min(g, 1.0f), 0.0f);
			b = std::max(std::min(b, 1.0f), 0.0f);
			alpha = std::max(std::min(alpha, 1.0f), 0.0f);
			color.emplace_back(DirectX::XMFLOAT4(r, g, b, alpha));
		}

		//pos
		position.emplace_back(DirectX::XMFLOAT3(point.pos[0], point.pos[1], point.pos[2]));
		depth_vec.emplace_back(std::pair(point.pos[2], depth_vec.size()));

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
			auto cov = DirectX::XMMatrixMultiply(transform_matrix_t, transform_matrix);
			cov3d.emplace_back(DirectX::XMFLOAT3X3(
				cov.r[0].m128_f32[0], cov.r[0].m128_f32[1], cov.r[0].m128_f32[2],
				cov.r[1].m128_f32[0], cov.r[1].m128_f32[1], cov.r[1].m128_f32[2],
				cov.r[2].m128_f32[0], cov.r[2].m128_f32[1], cov.r[2].m128_f32[2]
			));

			//AABB
			float coefficient = std::sqrt(2 * std::log(255 * alpha));
			auto axis = transform_matrix * coefficient; 
			DirectX::XMFLOAT3 extension{
			(std::abs(axis.r[0].m128_f32[0]) + std::abs(axis.r[1].m128_f32[0]) + std::abs(axis.r[2].m128_f32[0])),
			(std::abs(axis.r[0].m128_f32[1]) + std::abs(axis.r[1].m128_f32[1]) + std::abs(axis.r[2].m128_f32[1])),
			(std::abs(axis.r[0].m128_f32[2]) + std::abs(axis.r[1].m128_f32[2]) + std::abs(axis.r[2].m128_f32[2])),
			};
			AABB_extension.push_back(extension);

		}
	}
	buffer.reset();

	CreateCluster();


	return;
}
std::vector<uint64_t> GSLoader::GetMortonCode(const std::vector<DirectX::XMFLOAT3>& pos)
{
	//normalize
	DirectX::XMFLOAT3 min(flt::max(), flt::max(), flt::max());
	DirectX::XMFLOAT3 max(flt::min(), flt::min(), flt::min());
	for (const DirectX::XMFLOAT3& point_pos:pos)
	{
		min.x = std::min(min.x, point_pos.x);
		min.y = std::min(min.y, point_pos.y);
		min.z = std::min(min.z, point_pos.z);

		max.x = std::max(min.x, point_pos.x);
		max.y = std::max(min.y, point_pos.y);
		max.z = std::max(min.z, point_pos.z);
	}

	std::vector<uint64_t> morton_code(pos.size(), 0);
	constexpr uint32_t max_value_21bit = (1 << 21) - 1;
	for (int i=0;i<pos.size();i++)
	{
		const DirectX::XMFLOAT3& point_pos = pos[i];
		DirectX::XMFLOAT3 normalized_pos;
		normalized_pos.x = (point_pos.x - min.x) / max.x;
		normalized_pos.y = (point_pos.y - min.y) / max.y;
		normalized_pos.z = (point_pos.z - min.z) / max.z;

		uint32_t x = static_cast<uint32_t>(std::clamp(normalized_pos.x, 0.0f, 1.0f) * max_value_21bit);
		uint32_t y = static_cast<uint32_t>(std::clamp(normalized_pos.y, 0.0f, 1.0f) * max_value_21bit);
		uint32_t z = static_cast<uint32_t>(std::clamp(normalized_pos.z, 0.0f, 1.0f) * max_value_21bit);

		uint64_t code = 0;
		for (int j = 0; j < 21; ++j) {
			code |= (uint64_t)((x >> j) & 1) << (3 * j);     
			code |= (uint64_t)((y >> j) & 1) << (3 * j + 1); 
			code |= (uint64_t)((z >> j) & 1) << (3 * j + 2); 
		}
		morton_code[i] = code;
	}

	return morton_code;
}
void GSLoader::CreateCluster(int static_cluster_size)
{
	std::vector<uint64_t> morton_code = GetMortonCode(position);

	//sort index
	auto comp = [&morton_code](const int& a, const int& b) {
		if (morton_code[a] < morton_code[b]) return true;
		else return false;
		};
	std::vector<int> morton_index(morton_code.size(), 0);
	for (int i = 0; i < morton_code.size(); i++)
	{
		morton_index[i] = i;
	}
	std::sort(morton_index.begin(), morton_index.end(), comp);

	//reindex
	std::vector<DirectX::XMFLOAT3> position_cpy(position.begin(), position.end());
	std::vector<DirectX::XMFLOAT4> color_cpy(color.begin(), color.end());
	std::vector<DirectX::XMFLOAT3X3> cov3d_cpy(cov3d.begin(), cov3d.end());
	std::vector<DirectX::XMFLOAT3> AABB_extension_cpy(AABB_extension.begin(), AABB_extension.end());
	for (int i = 0; i < morton_index.size(); i++)
	{
		position[i] = position_cpy[morton_index[i]];
		color[i] = color_cpy[morton_index[i]];
		cov3d[i] = cov3d_cpy[morton_index[i]];
		AABB_extension[i] = AABB_extension_cpy[morton_index[i]];
	}

	int clusters_num = morton_index.size() / static_cluster_size;
	clusters.clear();
	cluster_AABB_origin.clear();
	cluster_AABB_extension.clear();
	for (int i = 0; i < clusters_num; i++)
	{
		std::vector<int> cluster_ele_index(static_cluster_size, 0);
		DirectX::XMFLOAT3 min{ flt::max(),flt::max(),flt::max() };
		DirectX::XMFLOAT3 max{ flt::min(),flt::min(),flt::min() };
		for (int cluster_i = 0; cluster_i < static_cluster_size; cluster_i++)
		{
			cluster_ele_index[cluster_i] = i * static_cluster_size + cluster_i;
			max.x = std::max(max.x, position[i * static_cluster_size].x + AABB_extension[i * static_cluster_size].x);
			min.x = std::min(max.x, position[i * static_cluster_size].x - AABB_extension[i * static_cluster_size].x);
			max.y = std::max(max.y, position[i * static_cluster_size].y + AABB_extension[i * static_cluster_size].y);
			min.y = std::min(max.y, position[i * static_cluster_size].y - AABB_extension[i * static_cluster_size].y);
			max.z = std::max(max.z, position[i * static_cluster_size].z + AABB_extension[i * static_cluster_size].z);
			min.z = std::min(max.z, position[i * static_cluster_size].z - AABB_extension[i * static_cluster_size].z);
		}
		cluster_AABB_origin.emplace_back(DirectX::XMFLOAT3{ (max.x+min.x)/2.0f,(max.y + min.y) / 2.0f ,(max.z + min.z) / 2.0f });
		cluster_AABB_extension.emplace_back(DirectX::XMFLOAT3{ (max.x - min.x) / 2.0f,(max.y - min.y) / 2.0f ,(max.z - min.z) / 2.0f });
	}

}