#pragma once
// 物品基类Object、纹理类Texture、碰撞点类HitPoint

#include "Vec3.h"
#include <cmath>
#include <string>
#include <vector>
#include <cstdlib>
#include <iostream>

/**
纹理类Texture，用于实现纹理贴图
*/
struct Texture
{
public:
	Texture(const std::string &fileName);
	Color colorUV(double u, double v) const;	// 在UV坐标系下，利用双线性插值算法获取(u, v)处的颜色值
	int rows, cols;

private:
	std::vector<std::vector<Vec3>> m_map;
};

/**
所有物品的基类：场景中所有物体均派生自此类，需要实现纯虚函数
Intersection intersect(const Vec3 &ori, const Vec3 &dir, double &maxDist, Vec3 *P, Vec3 *N, Color *color) const
*/
// 光线与物体相交的3种情形
enum Intersection 
{ 
	OUTSIDE = 1, 	// 从正面撞击
	INSIDE = -1,	// 从内部撞击（折射情况）
	MISS = 0		// 无交点
};
struct HitPoint;
class Object
{
public:
	Color color;	// 物体自身的颜色，如有纹理，则看到的为color与纹理的叠加色
	double diff, spec, refl, refr;	// 漫反射、高光、反射、折射各自所占的百分比，相加为1
	double ior;			// 折射率，默认为1.4
	Texture *texture;	// 物体的纹理，拷贝时纹理不复制，默认公用
	int id;				// 每个物体创建时确定的一个随机数，用于反走样时找出轮廓
	std::string name;	// 物体的名称，算法中不需要，仅用于调试
public:
	Object(const std::string &name_ = "") : name(name_), texture(NULL), id(rand()) {}
	~Object() {}

	void setMaterial(
		const Color &color_ = Color(), 
		double diff_ = 1, 
		double spec_ = 0,
		double refl_ = 0,
		double refr_ = 0,
		double ior_ = 1.4)	{
		color = color_;
		diff = diff_; spec = spec_; refl = refl_; refr = refr_;
		ior = ior_;
	}
	void setTexture(Texture *texture_) { 
		texture = texture_; 
	}

	// 判断物体与光线相交的情况：如果无交点/距离超过maxDist则返回MISS。
	// 如有碰撞，将碰撞点信息（如P、N等）存入备用
	virtual Intersection intersect(const Vec3 &ori, const Vec3 &dir, double &maxDist,
				 /*output*/ Vec3 *P = NULL, Vec3 *N = NULL, Color *objectColor = NULL) const = 0;
};

/**
碰撞点类HitPoint，用途有2：
1. 在RayTracing中，用于存储碰撞点信息，并在各个子函数之间传递参数；
2. 获取碰撞点图（建立KD树），用于PPM光子映射时计算此处的光通量。
*/
struct HitPoint
{
	Vec3 P, N;		// 碰撞点的位置、法向量
	Object *object;	// 该碰撞点位于的物体
	int row, col; 	// 碰撞点对应的屏幕坐标(row, col)
	Color weight;	// 计算此碰撞点处的色光权值，乘以累计的光通量，乘以一个系数后为最终颜色值
	Color phi;		// 本碰撞点处的累计光通量
	double radius2, maxRadius2;	// 本碰撞点的最大半径、本子树下的最大半径
	double nAccum, nNew;	// 对应于论文中的N、M：之前的累计光子数、本轮新增光子数

	HitPoint() {}
	HitPoint(int row_, int col_, const Color &weight_)
		: row(row_), col(col_), weight(weight_),
		  nAccum(0), nNew(0), phi(Color(0, 0, 0)) {}

	void update(double a) {	// a为论文中的α值
		if (nAccum <= 0 || nNew <= 0) return;	
		double k = (nAccum + a * nNew) / (nAccum + nNew);	// 半径衰减速率
		radius2 *= k; phi *= k;		// 半径缩小、光通量与面积成比例地缩小
		nAccum += a * nNew; nNew = 0;	// 更新累计光子数、清零新增光子数
	}
};