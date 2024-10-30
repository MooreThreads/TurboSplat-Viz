#include"gaussian_loader.h"
#include<fstream>
#include"bvh.h"

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
	int point_num = std::min((file_size - header_size) / point_size, 5 * 1024 * 1024);
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

	BuildBVH(static_cluster_size);



	return;
}

void GSLoader::BuildBVH(int static_cluster_size)
{
	//find global 
	std::unique_ptr<BVHManager> bvh = std::make_unique<BVHManager>(static_cluster_size);
	std::vector<AABB> gaussian_aabb;
	gaussian_aabb.reserve(position.size());
	for (int i = 0; i < position.size(); i++)
	{
		gaussian_aabb.emplace_back(AABB{ position[i].x ,position[i].y ,position[i].z ,
			AABB_extension[i].x,AABB_extension[i].y,AABB_extension[i].z  });
	}
	bvh->Build(gaussian_aabb);

	std::vector<AABB> cluster_aabb;
	bvh->GetCluster(clusters, cluster_aabb);
	for (const AABB& aabb : cluster_aabb)
	{
		cluster_AABB_origin.emplace_back(DirectX::XMFLOAT3{ aabb.origin[0],aabb.origin[1] ,aabb.origin[2] });
		cluster_AABB_extension.emplace_back(DirectX::XMFLOAT3{ aabb.extension[0],aabb.extension[1] ,aabb.extension[2] });
	}

}