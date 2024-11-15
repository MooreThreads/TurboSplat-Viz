#include"bvh.h"
#include<algorithm>
#include<assert.h>

AABB::AABB():origin{0,0,0},extension{-1,-1,-1}
{

}
AABB::AABB(const std::initializer_list<float>& init_list) 
{
	origin[0] = *(init_list.begin() + 0);
	origin[1] = *(init_list.begin() + 1);
	origin[2] = *(init_list.begin() + 2);
	extension[0] = *(init_list.begin() + 3);
	extension[1] = *(init_list.begin() + 4);
	extension[2] = *(init_list.begin() + 5);

}

AABB AABB::Union(const AABB& a, const AABB& b)
{
	if (a.extension[0] < 0)
	{
		return b;
	}
	if (b.extension[0] < 0)
	{
		return a;
	}

	float min[3];
	float max[3];
	for (int dim = 0; dim < 3; dim++)
	{
		min[dim] = std::min(a.origin[dim] - a.extension[dim], b.origin[dim] - b.extension[dim]);
		max[dim] = std::max(a.origin[dim] + a.extension[dim], b.origin[dim] + b.extension[dim]);
	}
	AABB ret;
	for (int dim = 0; dim < 3; dim++)
	{
		ret.extension[dim] = (max[dim] - min[dim]) * 0.5f;
		ret.origin[dim] = (max[dim] + min[dim]) * 0.5f;
	}
	return ret;
}

BVHNode::BVHNode():aabb(), childs()
{

}

void BVHNode::Add(std::shared_ptr<BVHNode> node)
{
	this->aabb = AABB::Union(node->aabb, this->aabb);
	this->childs.push_back(node);
	return;
}

BVHManager::BVHManager(int chunk_size):phead(nullptr), leaf2primitive()
{
	this->chunk_size = chunk_size;
}

bool BVHManager::RecursiveBuild(std::shared_ptr<BVHNode> cur_node)
{
	int childs_num = cur_node->childs.size();
	if (childs_num <= chunk_size)
	{
		return false;
	}

	//select dim and sort
	float max_extension = 0;
	int max_ext_dim = -1;
	for (int i = 0; i < 3; i++)
	{
		if (cur_node->aabb.extension[i] > max_extension)
		{
			max_ext_dim = i;
			max_extension = cur_node->aabb.extension[i];
		}
	}
	std::sort(cur_node->childs.begin(), cur_node->childs.end(), [max_ext_dim](const std::shared_ptr<BVHNode>& a, const std::shared_ptr<BVHNode>& b) {
		return a->aabb.origin[max_ext_dim] > b->aabb.origin[max_ext_dim];
		});

	float min_cost = 1e9f;
	int min_slot = -1;
	int slot_size = std::max((cur_node->childs.size() / 10) / chunk_size * chunk_size,(size_t)chunk_size);
	float cur_cost = 0.0f;
	for (int i = slot_size; i < cur_node->childs.size(); i += slot_size)
	{
		cur_cost = 0.0f;
		cur_cost += GetCost(cur_node.get(), cur_node->childs, 0, i );
		cur_cost += GetCost(cur_node.get(), cur_node->childs, i , childs_num);
		if (cur_cost < min_cost)
		{
			min_slot = i;
			min_cost = cur_cost;
		}
	}
	assert(min_slot > 0);
	std::shared_ptr<BVHNode> left = std::make_shared<BVHNode>();
	for (int i = 0; i < min_slot ; i++)
	{
		left->Add(cur_node->childs[i]);
	}
	std::shared_ptr<BVHNode> right = std::make_shared<BVHNode>();
	for (int i = min_slot ; i < childs_num; i++)
	{
		right->Add(cur_node->childs[i]);
	}
	cur_node->childs.clear();

	if (RecursiveBuild(left))
	{
		cur_node->childs.insert(cur_node->childs.end(), left->childs.begin(), left->childs.end());
		left.reset();
	}
	else
	{
		cur_node->childs.push_back(left);
	}

	if (RecursiveBuild(right))
	{
		cur_node->childs.insert(cur_node->childs.end(), right->childs.begin(), right->childs.end());
		right.reset();
	}
	else
	{
		cur_node->childs.push_back(right);
	}

	return true;
}

float BVHManager::GetCost(const BVHNode* cur_node, const std::vector<std::shared_ptr<BVHNode>>& sorted_childs, int start, int end)
{
	assert(start != end);
	
	float area_total = cur_node->aabb.extension[0] * cur_node->aabb.extension[1] * cur_node->aabb.extension[2];
	
	AABB sub=sorted_childs[start]->aabb;
	for (int i = start + 1; i < end; i++)
	{
		sub = AABB::Union(sub, sorted_childs[i]->aabb);
	}
	float sub_area = sub.extension[0] * sub.extension[1] * sub.extension[2];

	float prob = sub_area / area_total;
	float N = end - start;
	assert(std::_Is_nan(prob)==false);

	return prob * N;
}

void BVHManager::GetClusterInternel(const std::shared_ptr<BVHNode>& node, std::vector<std::vector<int>>& cluster, std::vector<AABB>& cluster_aabb)
{
	assert(node->childs.size() > 0);//not leaf
	if (node->childs[0]->childs.size() > 0)
	{
		for (const auto& child_node : node->childs)
		{
			GetClusterInternel(child_node, cluster, cluster_aabb);
		}
	}
	else
	{
		std::vector<int> cur_cluster;
		for (const auto& child_node : node->childs)
		{
			cur_cluster.push_back(leaf2primitive[child_node.get()]);
		}
		cluster.emplace_back(cur_cluster);
		cluster_aabb.push_back(node->aabb);
	}
	
}

void BVHManager::GetCluster(std::vector<std::vector<int>>& cluster, std::vector<AABB>& cluster_aabb)
{
	GetClusterInternel(phead, cluster, cluster_aabb);
}

void BVHManager::Build(const std::vector<AABB>& aabb_list)
{
	phead.reset();
	phead = std::make_shared<BVHNode>();
	for (int i = 1; i < aabb_list.size(); i++)
	{
		std::shared_ptr<BVHNode> node = std::make_shared<BVHNode>();
		node->aabb = aabb_list[i];
		this->leaf2primitive[node.get()] = i;
		phead->Add(node);
	}
	RecursiveBuild(phead);
}