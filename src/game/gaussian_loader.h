#include<string>
#include<DirectXMath.h>
#include<vector>
class GSLoader
{
private:
	void CreateCluster(int static_cluster_size=64);
	std::vector<uint64_t> GetMortonCode(const std::vector<DirectX::XMFLOAT3>& pos);
public:
	std::vector<DirectX::XMFLOAT3> position;
	std::vector<DirectX::XMFLOAT4> color;
	std::vector<DirectX::XMFLOAT3X3> cov3d;
	std::vector<DirectX::XMFLOAT3> AABB_extension;

	std::vector<std::vector<int>> clusters;
	std::vector<DirectX::XMFLOAT3> cluster_AABB_origin;
	std::vector<DirectX::XMFLOAT3> cluster_AABB_extension;


	void Load(std::string path,int static_cluster_size);
};