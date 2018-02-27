#pragma once
// 双三次贝塞尔曲面类Double3Bezier、包围盒类BoundingBox

#include "../Vec3.h"
#include "../Object.h"
#include <queue>
#include <Eigen/Dense>

typedef Eigen::Vector3d EVec3d;

// 双三次贝塞尔曲面类Double3Bezier类
class Node;
class Double3Bezier : public Object
{
public:
	EVec3d C;		// 控制曲面的位置，未必在中心
	EVec3d P[16];	// 曲面的16个控制点

public:
	Double3Bezier(const Vec3 &C_, EVec3d *P_, const std::string &name_ = "")
		: Object(name_) {
		C = EVec3d(C_[0], C_[1], C_[2]);
		for (int i = 0; i < 16; i++) P[i] = P_[i] + C;
		saveAsObj(P_);
	}

	virtual Intersection intersect(const Vec3 &ori, const Vec3 &dir, double &maxDist,
		/*output*/ Vec3 *P = NULL, Vec3 *N = NULL, Vec3 *objectColor = NULL) const;
	void saveAsObj(EVec3d *P_);

private:
	EVec3d Double3Bezier::newton(const EVec3d &ori, const EVec3d &dir, const Node &patch, int maxIter, int *nIter) const;
};

// 包围盒BoundingBox类，这里实现的是与坐标轴平行的包围盒AABB
struct BoundingBox
{
	EVec3d Pmax, Pmin;
	// 为双三次Bezier曲面建立包围盒
	BoundingBox(const EVec3d *P);
	// 无穷范数，为各维度坐标中绝对值最大者
	double infNorm() const { return (Pmax - Pmin).lpNorm<Eigen::Infinity>(); }
	// 判定光线是否与包围盒相交
	bool intersect(const EVec3d &ori, const EVec3d &dir);
};