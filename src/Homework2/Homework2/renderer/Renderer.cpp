#include "Renderer.h"
#include "../World.h"
#include "../Light.h"
#include "../Camera.h"

#include <omp.h>
#include <ctime>
#include <fstream>
#include <cstdlib>
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>
using namespace std;

// 顶层渲染接口，分为PASS1：光线追踪；PASS2：光子发射
void Renderer::render(World *world_)
{
	m_world = world_;
	int height = m_world->camera->height, width = m_world->camera->width;
	m_photo.resize(height);
	for (int i = 0; i < height; i++) m_photo[i].resize(width);
	for (int i = 0; i < height; i++) for (int j = 0; j < width; j++) m_photo[i][j] = Color();

	int startTime = clock();
	// PASS1: Ray Tracing
	for (int i = 0; i < height; i++) for (int j = 0; j < width; j++)
	{
		HitPoint hp(i, j, Vec3(1.0, 1.0, 1.0));
		// 有景深效果，则增加随机采样环节
		if (m_world->camera->aperture > EPSILON)
		{
			int nSample = m_world->camera->nSample;
			hp.weight /= nSample;
			for (int k = 0; k < nSample; k++)
			{
				auto ray = m_world->camera->rayAperture(i, j);
				Vec3 apertOri = ray.first, apertDir = ray.second;
				m_photo[i][j] += traceRay(hp, apertOri, apertDir, 0);
			}
			m_photo[i][j] /= nSample;
		} else	// 否则无景深，纯RT
		{
			Vec3 ori = m_world->camera->C;
			Vec3 dir = m_world->camera->ray(i, j);
			m_photo[i][j] = traceRay(hp, ori, dir, 0);
		}
	}
	cout << "Elapsed time: " << (clock() - startTime) / CLOCKS_PER_SEC << "s." << endl;
	this->saveImg("RT.jpg");


	// PASS2: Photon tracing
	m_kdMap.init(m_hitpoints.size(), m_hitpoints.data());	// 建立碰撞点图

	// 渐进式发射光子
	for (int i = 0; i < MAX_PPM_ITER; i++)
	{
		// 计算所有光源的能量和
		double totalPower = 0.0;
		for (Light *light : m_world->lights)
			totalPower += light->color.power();

		// 单个光子的能量：总能量/光子数
		double photonPower = totalPower / MAX_PHOTON_NUM;

		// 各个光源发射光子
		for (Light *light : m_world->lights)
		{
			int nPhoton = light->color.power() / photonPower;	// 本光源发射的光子数
			Vec3 photonColor = light->color / nPhoton;	// 本光源的光子能量

			// OpenMP多线程加速
			omp_set_dynamic(0);
			omp_set_num_threads(8);
#pragma omp parallel
			{
				srand(int(time(NULL)) ^ omp_get_thread_num());	// rand()线程不安全，需重新设定随机种子
#pragma omp for
				for (int j = 0; j < nPhoton; j++)
				{
					if (j % 100000 == 0) cout << "j = " << j << endl;
					// 在光源上随机选择光线始点、方向
					Vec3 ori = light->randomPoint();
					Vec3 dir = Vec3::random();
					Photon photon(ori, dir, photonColor);
					tracePhoton(photon, 0);
				}
			}
		}
		cout << "Elapsed time: " << (clock() - startTime) / CLOCKS_PER_SEC << "s." << endl;

		// 每一轮光子发射结束后，更新KdMap
		this->updateKDMap();

		// 估算辉度
		this->evalIrradiance(i + 1);

		// 保存图像
		this->saveImg("update.jpg");
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// PASS1
// 光线追踪，建立碰撞点图，此步之后m_photo中为RT的结果。传入的dir必须为单位向量
Vec3 Renderer::traceRay(HitPoint hp, const Vec3 &ori, const Vec3 &dir, int depth)
{
	// 递归基：超过最大递归深度
	if (depth > MAX_DEPTH) { m_bgHitpoints.push_back(hp); return m_world->bgColor; }

	// 寻找最近的相交物体，并获取法向量、碰撞位置、碰撞位置的颜色（与纹理有关）
	double maxDist = INT_MAX;
	Object *nearestObject = NULL;
	Vec3 P, N, objectColor;	// 碰撞位置、法向量、颜色
	Intersection intersection = MISS, temp = MISS;
	for (Object *object : m_world->objects)
		if ((temp = object->intersect(ori, dir, maxDist, &P, &N, &objectColor)))
			nearestObject = object, intersection = temp;

	// 递归基：无碰撞
	if (!nearestObject) { m_bgHitpoints.push_back(hp); return m_world->bgColor; }

	// 分别计算漫反射&高光（Phong模型）、镜面反射、折射
	Vec3 ret(0, 0, 0);
	//////////////////////////////////////////////////////////////// 漫反射&高光
	if (nearestObject->diff > EPSILON || nearestObject->spec > EPSILON)	
	{
		// Non-specular表面：存储HitPoint
		HitPoint hpDiff = hp;
		hpDiff.object = nearestObject; hpDiff.P = P; hpDiff.N = N;
		hpDiff.weight *= objectColor * nearestObject->diff;
		hpDiff.radius2 = INIT_RADIUS * INIT_RADIUS;
		m_hitpoints.push_back(hpDiff);

		// 计算Phong模型
		for (Light *light : m_world->lights)
		{
			// 判断阴影
			bool visible = true;
			Vec3 L = light->C - P;				// 通往光源的向量
			double objectDist = L.length();		// 到nearestObject的距离
			for (Object *object : m_world->objects)
				if (object != nearestObject 
				 && object->intersect(P, L.normalized(), objectDist))
					{ visible = false; break; }
			if (!visible) continue;

			// 计算Phong模型
			L = L.normalized();
			Vec3 V = (ori - P).normalized();
			ret += light->phong(N, L, V, nearestObject->diff, nearestObject->spec, objectColor);
		}
	} 
	//////////////////////////////////////////////////////////////// 镜面反射
	if (nearestObject->refl > EPSILON)	
	{
		HitPoint hpRefl = hp;
		hpRefl.weight *= objectColor * nearestObject->refl;
		ret += traceRay(hpRefl, P, dir.reflected(N), depth + 1) * nearestObject->refl * objectColor;
	}
	//////////////////////////////////////////////////////////////// 折射
	if (nearestObject->refr > EPSILON)
	{
		HitPoint hpRefr = hp;
		hpRefr.weight *= objectColor * nearestObject->refr;
		double n = (intersection == INSIDE) ? nearestObject->ior : (1 / nearestObject->ior);
		Vec3 refracted = dir.refracted((intersection == INSIDE ? -N : N), n);
		ret += traceRay(hpRefr, P, refracted, depth + 1) * nearestObject->refr * objectColor;
	}
	return ret;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// PASS2
// 光子发射，让光子纷纷扬扬地洒向场景~~~~美哉幻哉，美哉幻哉
void Renderer::tracePhoton(Photon &photon, int depth)
{
	// 递归基：超过最大追踪深度
	if (depth > MAX_DEPTH) return;

	// 寻找最近的被光子打中的物体，并获取法向量、碰撞位置、碰撞位置的颜色（与纹理有关）
	double maxDist = INT_MAX;
	Object *nearestObject = NULL;
	Vec3 P, N, objectColor;
	Intersection intersection = MISS, temp = MISS;
	for (Object *object : m_world->objects)
		if ((temp = object->intersect(photon.ori, photon.dir, maxDist, &P, &N, &objectColor)))
			nearestObject = object, intersection = temp;

	// 递归基：无碰撞物体
	if (!nearestObject) return;

	// 根据被碰撞的物体，更新光子信息
	photon.P = P; photon.object = nearestObject;

	// 如果是漫反射表面，查询该光子在哪些碰撞点的内部，存入kdMap中
	if (nearestObject->diff > EPSILON) m_kdMap.insertPhoton(photon);

	// 准备发射新光子
	Photon newPhoton = photon;
	newPhoton.color *= objectColor; newPhoton.ori = P;

	// 轮盘赌
	double estimater = rand01();
	double mark1 = nearestObject->diff + nearestObject->spec,
		   mark2 = mark1 + nearestObject->refl,
		   mark3 = mark2 + nearestObject->refr;
	if (estimater < mark1)	// diff + spec
	{
		newPhoton.dir = Vec3::randomCosine(N);
		tracePhoton(newPhoton, depth + 1);
	} else if (estimater < mark2)	// refl
	{
		newPhoton.dir = photon.dir.reflected(N);
		tracePhoton(newPhoton, depth + 1);
	} else // refr
	{
		double n = (intersection == INSIDE) ? nearestObject->ior : (1 / nearestObject->ior);
		newPhoton.dir = photon.dir.refracted((intersection == INSIDE) ? -N : N, n);
		tracePhoton(newPhoton, depth + 1);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// 更新碰撞点图
void Renderer::updateKDMap()
{
	int nHitpoint = 0;
	HitPoint *hitpoints = m_kdMap.data(/*&*/nHitpoint);

	for (int i = 0; i < nHitpoint; i++) hitpoints[i].update(ALPHA);
	m_kdMap.update();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// 估算辉度
void Renderer::evalIrradiance(int nIter)
{
	// 将光线追踪的颜色值清零
	int height = m_world->camera->height, width = m_world->camera->width;
	for (int i = 0; i < height; i++) for (int j = 0; j < width; j++) m_photo[i][j] = Vec3(0, 0, 0);

	// 估算辉度
	int nHitpoint = 0;
	HitPoint *hitpoints = m_kdMap.data(/*&*/nHitpoint);
	for (int i = 0; i < nHitpoint; i++)
	{
		HitPoint hp = hitpoints[i];
		Vec3 irradiance = 10000.0 * hp.phi / (hp.radius2 * nIter);
		m_photo[hp.row][hp.col] += irradiance * hp.weight;
	}

	// 计入背景色
	Vec3 bgColor = m_world->bgColor;
	for (int i = 0; i < m_bgHitpoints.size(); i++)
	{
		HitPoint hp = m_bgHitpoints[i];
		m_photo[hp.row][hp.col] += bgColor * hp.weight;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// 保存图片
void Renderer::saveImg(const string &fileName)
{
	cv::Mat_<cv::Vec3b> img;
	int height = m_world->camera->height, width = m_world->camera->width;
	img.create(height, width);

	for (int i = 0; i < height; i++) for (int j = 0; j < width; j++)
		for (int k = 0; k < 3; k++)
			img(i, j)[k] = min(m_photo[i][j][2 - k], 1.0) * 255;

	cv::imwrite(fileName, img);
}