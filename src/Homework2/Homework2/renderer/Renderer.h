#pragma once
// 渲染器类Renderer

#include "../Object.h"
#include "utils.h"
#include <vector>

class World;
/**
渲染器类Renderer，负责实现光线追踪、渐进式光子映射算法
*/
class Renderer
{
public:
	const static int MAX_DEPTH = 8;	// 光线追踪、光子映射的最大深度
	const static int MAX_PPM_ITER = 100000;	// PPM最大迭代次数
	const static int MAX_PHOTON_NUM = 5000000;	// 最大发射光子数
	const double INIT_RADIUS;	// 各个碰撞点初始半径
	const double ALPHA;	// 论文中的系数α，决定半径衰减速率

public:
	Renderer() : INIT_RADIUS(2), ALPHA(0.5) {}	// 调参，场景大小为200左右时较为合适
	~Renderer() {}

	// 主要接口，渲染顶层调用
	void render(World *world);	
	// PASS1：光线追踪，建立碰撞点图（调试时，将PASS2以下的代码全部注释掉，即得纯RT）
	Color traceRay(HitPoint hp, const Vec3 &ori, const Vec3 &dir, int depth);
	// PASS2：光子发射，查询、更新碰撞点图
	void tracePhoton(Photon &photon, int depth);
	// 将渲染好的图片存入文件（每发射一轮光子就保存一次）
	void saveImg(const std::string &fileName);

private:
	// 内部接口：根据本次发射的光子更新碰撞点图
	void updateKDMap();
	// 内部接口：根据场景中光子密度分布，估算各像素辉度
	void evalIrradiance(int nIter);

private:
	World *m_world;
	std::vector<std::vector<Vec3>> m_photo;
	std::vector<HitPoint> m_hitpoints;
	std::vector<HitPoint> m_bgHitpoints;
	KDMap m_kdMap;
};