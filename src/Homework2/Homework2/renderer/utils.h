#pragma once
// 光子类Photon，KD树碰撞点图类KDMap
#include "../Object.h"

/**
光子类Photon，用于实现光子映射，其在空间中的密度分布决定了光照分布
*/
class Photon
{
public:
	Vec3 ori, dir, P;	// 光子发射始点、方向、落点
	Color color; // 单个光子携带的能量，由光源能量、光子数共同决定
	Object *object; // 附着的物体
	Photon(const Vec3 &ori_, const Vec3 &dir_, const Color &color_)
		: ori(ori_), dir(dir_), color(color_) {}
};

/**
碰撞点图类KDMap，用于保存碰撞点的数据结构，
每发射一个光子p，就在碰撞点图中查询：“p在哪些碰撞点的半径之内？”对查询得到的所有碰撞点累计该光子携带的能量
KDMap的实现，有部分参考了http://www.cnblogs.com/eyeszjwang/articles/2429382.html中的伪代码，并结合实验情景加以修改
*/
class KDMap
{
	static const int K = 3;
	struct Node
	{
		int value;
		int split;
		Node *left, *right;
	} *m_memory;	// 数组实现

private:
	Node *m_root;	// 根节点
	int m_size;		// 节点数目
	int *m_index;	// 坐标对应（因为是数组实现）
	int m_nNode;	
	HitPoint *m_data;

	void medianPartition(int l, int r, int dim, int k); //getMedian
	Node* build(int l, int r, double *min, double *max);//递归建树
	void insertPhoton(Node *node, const Photon &photon);//递归加入光子
	void update(Node *node);

public:
	~KDMap();
	
	HitPoint *data(int &_size) const { _size = m_size; return m_data; }
	void init(int _size, HitPoint *_data) { load(_size, _data); build(); }
	void load(int _size, HitPoint *_data); 
	void build(); 
	void insertPhoton(const Photon &photon);
	void update();
}; 