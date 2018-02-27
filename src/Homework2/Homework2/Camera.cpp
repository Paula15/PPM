#include "Camera.h"
using namespace std;

const Vec3 	 Camera::DEFAULT_C = Vec3(-50, 300, -20);
const double Camera::DEFAULT_LENS = 2800;
const int 	 Camera::DEFAULT_HEIGHT = 768;
const int 	 Camera::DEFAULT_WIDTH = 1024;
const double Camera::DEFAULT_APERTURE = 0;
const double Camera::DEFAULT_FOCAL_DIST = 90;
const int    Camera::DEFAULT_SAMPLE_NUM = 10;

// 将相机以shiftH_, shiftW_的偏移量对准场景中的P点
void Camera::lookAt(const Vec3 &P, int shiftH_, int shiftW_, double scale)
{
	const Vec3 up(0, 1, 0);	// 定义的“向上”的方向
	F = (P - C).normalized() * DEFAULT_LENS * scale;	// 获取镜头焦距向量
	W = cross(F, up).normalized() * DEFAULT_WIDTH * scale;	// 获得与焦距向量垂直的向量作为W标架
	H = -cross(W, F).normalized() * DEFAULT_HEIGHT * scale;	// 获得与F、W都垂直的向量作为H标架
	focalDist = C.z - P.z;	// 将P点置于景深焦平面上
	shiftH = shiftH_ * scale; shiftW = shiftW_ * scale;
	height *= scale; width *= scale;
}

// 获取射向屏幕中(h, w)像素的光线单位向量，不考虑景深。h, w可能不是整数（如超采样抗锯齿）
Vec3 Camera::ray(double h, double w) const
{
	h += shiftH; w += shiftW;
	return (F + H * (2 * h / height - 1) + W * (2 * w / width - 1)).normalized();
}

// 获取射向屏幕中(h, w)像素的光线起点和方向，考虑景深
Camera::Ray Camera::rayAperture(double h, double w) const
{
	h += shiftH; w += shiftW;
	// 光线初始方向、焦平面上物体位置
	Vec3 emitDir = F + H * (2 * h / height - 1) + W * (2 * w / width - 1);
	Vec3 target = C + emitDir * focalDist / -emitDir.z;

	// 在光圈上随机采样，获得随机偏移量detH, detW
	double detH, detW;
	do { detH = rand01() * 2.0 - 1.0; detW = rand01() * 2.0 - 1.0; } while (detH * detH + detW * detW >= 1.0);
	Vec3 unitH = H.normalized(), unitW = W.normalized();

	// 返回本次随机采样获得的光线：对光线起点加以随机扰动，但保证焦平面上物体清晰
	Vec3 ori = C + (unitH * detH + unitW * detW) * aperture;
	Vec3 dir = (target - ori).normalized();
	return make_pair(ori, dir);
}