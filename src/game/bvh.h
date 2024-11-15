#include<set>
#include<vector>
#include<map>
#include<memory>

struct AABB
{
	float extension[3];
	float origin[3];
	AABB();
	AABB(const std::initializer_list<float>& init_list);
	static AABB Union(const AABB& a, const AABB& b);
};

class BVHNode 
{
public:
	AABB aabb;
	BVHNode();


	std::vector<std::shared_ptr<BVHNode>> childs;
	void Add(std::shared_ptr<BVHNode> node);

};

class BVHManager
{
private:
	std::shared_ptr<BVHNode> phead;
	std::map<BVHNode*, int> leaf2primitive;
	int chunk_size;
	bool RecursiveBuild(std::shared_ptr<BVHNode> cur_node);
	float GetCost(const BVHNode* cur_node, const std::vector<std::shared_ptr<BVHNode>>& sorted_childs, int start, int end);
	void GetClusterInternel(const std::shared_ptr<BVHNode>& node, std::vector<std::vector<int>>& cluster, std::vector<AABB>& cluster_aabb);
	
public:
	BVHManager(int chunk_size);
	void Build(const std::vector<AABB>& aabb_list);
	void GetCluster(std::vector<std::vector<int>>& cluster, std::vector<AABB>& cluster_aabb);
};