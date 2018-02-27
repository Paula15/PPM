#pragma once
// 球体类Sphere

#include "../Object.h"

class Sphere : public Object
{
public:
	Vec3 C;   // 球心坐标
	double R; // 半径
	Vec3 texU, texV; // 用于纹理贴图时计算UV坐标
	const static Vec3 DEFAULT_TEXU;	// texU, texV的默认值
	const static Vec3 DEFAULT_TEXV;

public:
	Sphere(const Vec3 &C_, double R_, const std::string &name_ = "")
		: Object(name_), C(C_), R(R_) {
			texU = DEFAULT_TEXU.normalized();
			texV = DEFAULT_TEXV.normalized();
		}
	virtual Intersection intersect(const Vec3 &ori, const Vec3 &dir, double &maxDist,
				/*output*/ Vec3 *P = NULL, Vec3 *N = NULL, Color *objectColor = NULL) const;
	// 计算P点处的纹理颜色
	Color texColor(const Vec3 &P) const;
};