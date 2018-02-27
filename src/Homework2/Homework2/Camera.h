#pragma once
// 摄像机类Camera

#include "Vec3.h"

/**
摄像机类Camera，负责发射光线、计算景深等
关键参数：位置、焦距、方向、图片尺寸、光圈、焦平面
*/
class Camera
{
public:
	Vec3 C;  				// 相机位置
	Vec3 F, H, W;	    	// 镜头朝向、镜头方向标架(H, W)
	int height, width;		// 照片的尺寸
	int shiftH, shiftW; 	// 镜头在竖直、水平方向上的平移

	double aperture;    // 光圈大小
	double focalDist;   // 焦平面距离
	int nSample;   // 随机采样次数

	static const Vec3 DEFAULT_C;		// 默认的相机位置
	static const double DEFAULT_LENS;	// 默认的焦距
	static const int DEFAULT_HEIGHT;	// 默认的图片高
	static const int DEFAULT_WIDTH;		// 默认的图片宽
	static const double DEFAULT_APERTURE;	// 默认的光圈大小
	static const double DEFAULT_FOCAL_DIST; // 默认的焦平面距离
	static const int DEFAULT_SAMPLE_NUM;	// 默认的随机采样次数
	typedef std::pair<Vec3, Vec3> Ray;

public:
	Camera(const Vec3 &C_ = DEFAULT_C, 
		   double aperture_ = DEFAULT_APERTURE,
		   double focalDist_ = DEFAULT_FOCAL_DIST,
		   double nSample_ = DEFAULT_SAMPLE_NUM) 
		   : C(C_), height(DEFAULT_HEIGHT), width(DEFAULT_WIDTH),
		     aperture(aperture_), focalDist(focalDist_), nSample(nSample_) {}

	// 方便地让相机对准场景中的某一个点
	void lookAt(const Vec3 &P, int shiftH_ = 0, int shiftW_ = 0, double scale = 1.0);
	// 获取穿过屏幕某一个点(h, w)的光线
	Vec3 ray(double h, double w) const;
	// 用于景深，随机采样，获取穿过屏幕上(h, w)的光线，返回光线出发点、方向
	Ray rayAperture(double h, double w) const;
};