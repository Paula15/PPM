#pragma once
// 光源类Light

#include "Vec3.h"

// 光源基类，所有光源必须实现纯虚函数phong的计算
class Light
{
public:
	Vec3 C;		 // 光源位置
	Color color; // 发光颜色

public:
	Light(const Vec3 &C_, const Color &color_) 
		: C(C_), color(color_) {}

	// 在光源上随机获得一个采样点，适用于面光源/线光源
	virtual Vec3 randomPoint() const = 0;

	// Phong模型
	virtual Color phong(
		const Vec3 &N, 				  // 法向量
		const Vec3 &L, const Vec3 &V, // 到光源的向量、到视点的向量
		double diff, double spec,	  // 漫反射、高光系数
		const Color &objectColor
	) const = 0;
};

// 点光源类
class PointLight : public Light
{
public:
	PointLight(const Vec3 &C_, const Color &color_)
		: Light(C_, color_) {}

	virtual Vec3 randomPoint() const { return C; }

	virtual Color phong(
		const Vec3 &N, 
		const Vec3 &L, const Vec3 &V, 
		double diff, double spec,
		const Color &objectColor
	) const;
};