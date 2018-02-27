#include "Double3Bezier.h"
#include <queue>
#include <fstream>
#include <memory>
using namespace std;

namespace Bezier {
	EVec3d curvePoint(const EVec3d *P, double t);	// 在3次贝塞尔曲线上参数为t的点
	EVec3d curveDeriv(const EVec3d *P, double t);	// 在3次贝塞尔曲线上参数为t处的切向量
	EVec3d patchPoint(const EVec3d *P, double u, double v);	// 在双3次贝塞尔曲面上参数(u, v)的点
	EVec3d patchDerivU(const EVec3d *P, double u, double v);	// 在双3次贝塞尔曲面上参数(u, v)处U方向的切向量
	EVec3d patchDerivV(const EVec3d *P, double u, double v);	// 在双3次贝塞尔曲面上参数(u, v)处V方向的切向量
	void  patchSplit(const EVec3d *P, /*output*/EVec3d *Pchildren); // 将双3次贝塞尔曲面插值四分（新曲面接口平滑）
}

/////////////////////////////////////////////////////////////////////
// Bezier
using namespace Bezier;
// 在3次贝塞尔曲线上参数为t的点，P[]的大小必须为4
EVec3d Bezier::curvePoint(const EVec3d *P, double t)
{
	double coeff0 = (1 - t) * (1 - t) * (1 - t);
	double coeff1 = 3 * t * (1 - t) * (1 - t);
	double coeff2 = 3 * t * t * (1 - t);
	double coeff3 = t * t * t;
	return P[0] * coeff0 + P[1] * coeff1 + P[2] * coeff2 + P[3] * coeff3;
}

// 在3次贝塞尔曲线上参数为t处的切向量，P[]的大小必须为4
EVec3d Bezier::curveDeriv(const EVec3d *P, double t)
{
	double coeff0 = -3 * (1 - t) * (1 - t);
	double coeff1 = 3 * (1 - t) * (1 - t) - 6 * t * (1 - t);
	double coeff2 = 6 * t * (1 - t) - 3 * t * t;
	double coeff3 = 3 * t * t;
	return P[0] * coeff0 + P[1] * coeff1 + P[2] * coeff2 + P[3] * coeff3;
}

// 在双3次贝塞尔曲面上参数(u, v)的点，P[]的大小必须为16
EVec3d Bezier::patchPoint(const EVec3d *P, double u, double v)
{
	EVec3d curve[4];
	for (int i = 0; i < 4; i++)	curve[i] = curvePoint(P + 4 * i, u);
	return curvePoint(curve, v);
}

// 在双3次贝塞尔曲面上参数(u, v)处U方向的切向量，P[]的大小必须为16
EVec3d Bezier::patchDerivU(const EVec3d *P, double u, double v)
{
	EVec3d Pv[4];
	EVec3d curve[4];
	for (int i = 0; i < 4; i++)
	{
		Pv[0] = P[i];
		Pv[1] = P[i + 4];
		Pv[2] = P[i + 8];
		Pv[3] = P[i + 12];
		curve[i] = curvePoint(Pv, v);
	}
	return curveDeriv(curve, u);
}

// 在双3次贝塞尔曲面上参数(u, v)处V方向的切向量，P[]的大小必须为16
EVec3d Bezier::patchDerivV(const EVec3d *P, double u, double v)
{
	EVec3d curve[4];
	for (int i = 0; i < 4; i++) curve[i] = curvePoint(P + 4 * i, u);
	return curveDeriv(curve, v);
}

// 将双3次贝塞尔曲面插值四分（新曲面接口平滑），P[]的大小必须为16
// 输出的数组Pchildren大小为64: [0, 16), [16, 32),[32, 48), [48, 64)是分裂出的4个子曲面控制点
void Bezier::patchSplit(const EVec3d *P, /*output*/EVec3d *Pchildren)
{
	// 原曲面首先分裂为L, R两块子曲面
	EVec3d L[16], R[16];
	// l, r再分裂为LL, LR, RL, RR四块子曲面，得到分裂结果
	EVec3d LL[16], LR[16], RL[16], RR[16];

	// 第一次分裂：沿着一条水平线分裂，得到L，R上下2曲面
	for (int iv = 0; iv < 4; iv++)
	{
		// 沿着V方向竖直的4个控制点
		EVec3d P0 = P[iv];
		EVec3d P1 = P[iv + 4];
		EVec3d P2 = P[iv + 8];
		EVec3d P3 = P[iv + 12];

		// L（上半曲面）插值
		L[iv] = P0;
		L[iv + 4] = P0 / 2 + P1 / 2;
		L[iv + 8] = P0 / 4 + P1 / 2 + P2 / 4;
		L[iv + 12] = P0 / 8 + P1 * 3.0 / 8 + P2 * 3.0 / 8 + P3 / 8;

		// R（下半曲面）插值
		R[iv] = L[iv + 12];
		R[iv + 4] = P1 / 4 + P2 / 2 + P3 / 4;
		R[iv + 8] = P2 / 2 + P3 / 2;
		R[iv + 12] = P3;
	}

	// 第二次分裂：上曲面沿着一条竖直线分裂，得到LL，LR左右2曲面
	for (int iv = 0; iv < 4; iv++)
	{
		// 沿着U方向水平的4个控制点
		EVec3d P0 = L[4 * iv];
		EVec3d P1 = L[4 * iv + 1];
		EVec3d P2 = L[4 * iv + 2];
		EVec3d P3 = L[4 * iv + 3];

		// LL（左半曲面）插值
		LL[4 * iv] = P0;
		LL[4 * iv + 1] = P0 / 2 + P1 / 2;
		LL[4 * iv + 2] = P0 / 4 + P1 / 2 + P2 / 4;
		LL[4 * iv + 3] = P0 / 8 + P1 * 3.0 / 8 + P2 * 3.0 / 8 + P3 / 8;

		// LR（右半曲面）插值
		LR[4 * iv] = LL[4 * iv + 3];
		LR[4 * iv + 1] = P1 / 4 + P2 / 2 + P3 / 4;
		LR[4 * iv + 2] = P2 / 2 + P3 / 2;
		LR[4 * iv + 3] = P3;
	}

	// 第二次分裂：下曲面沿着一条竖直线分裂，得到RL，RR左右2曲面
	for (int iv = 0; iv < 4; iv++)
	{
		// 沿着U方向水平的4个控制点
		EVec3d P0 = R[4 * iv];
		EVec3d P1 = R[4 * iv + 1];
		EVec3d P2 = R[4 * iv + 2];
		EVec3d P3 = R[4 * iv + 3];

		// LL（左半曲面）插值
		RL[4 * iv] = P0;
		RL[4 * iv + 1] = P0 / 2 + P1 / 2;
		RL[4 * iv + 2] = P0 / 4 + P1 / 2 + P2 / 4;
		RL[4 * iv + 3] = P0 / 8 + P1 * 3.0 / 8 + P2 * 3.0 / 8 + P3 / 8;

		// LR（右半曲面）插值
		RR[4 * iv] = RL[4 * iv + 3];
		RR[4 * iv + 1] = P1 / 4 + P2 / 2 + P3 / 4;
		RR[4 * iv + 2] = P2 / 2 + P3 / 2;
		RR[4 * iv + 3] = P3;
	}

	// 将LL、LR、RL、RR打包返回
	for (int i = 0; i < 16; i++)
	{
		Pchildren[i] = LL[i];
		Pchildren[i + 16] = LR[i];
		Pchildren[i + 32] = RL[i];
		Pchildren[i + 48] = RR[i];
	}
}
/////////////////////////////////////////////////////////////////////
// BoundingBox
BoundingBox::BoundingBox(const EVec3d *P)
{
	Pmin = EVec3d(INT_MAX, INT_MAX, INT_MAX); Pmax = -Pmin;
	for (int i = 0; i < 16; i++) for (int j = 0; j < 3; j++)
		Pmax(j) = max(Pmax(j), P[i](j)), Pmin(j) = min(Pmin(j), P[i](j));
}

// 包围盒求交算法（本段代码部分参考网络资源实现）
bool BoundingBox::intersect(const EVec3d &ori, const EVec3d &dir)
{
	const double eps = 1e-6;
	double ox = ori(0), oy = ori(1), oz = ori(2);
	double dx = dir(0), dy = dir(1), dz = dir(2);
	double x0 = Pmin(0), y0 = Pmin(1), z0 = Pmin(2);
	double x1 = Pmax(0), y1 = Pmax(1), z1 = Pmax(2);
	double tx_min = -RAND_MAX, ty_min = -RAND_MAX, tz_min = -RAND_MAX;
	double tx_max = RAND_MAX, ty_max = RAND_MAX, tz_max = RAND_MAX;
	//x0,y0,z0为包围体的最小顶点
	//x1,y1,z1为包围体的最大顶点
	if (fabs(dx) < eps) {
		//若射线方向矢量的x轴分量为0且原点不在盒体内
		if (ox < x0 || ox > x1) return false;
	}
	else {
		if (dx >= 0) {
			tx_min = (x0 - ox) / dx;
			tx_max = (x1 - ox) / dx;
		}
		else {
			tx_min = (x1 - ox) / dx;
			tx_max = (x0 - ox) / dx;
		}
	}
	if (fabs(dy) < eps) {
		//若射线方向矢量的x轴分量为0且原点不在盒体内
		if (oy < y0 || oy > y1) return false;
	}
	else {
		if (dy >= 0) {
			ty_min = (y0 - oy) / dy;
			ty_max = (y1 - oy) / dy;
		}
		else {
			ty_min = (y1 - oy) / dy;
			ty_max = (y0 - oy) / dy;
		}
	}
	if (fabs(dz) < eps) {
		//若射线方向矢量的x轴分量为0且原点不在盒体内
		if (oz < z0 || oz > z1) return false;
	}
	else {
		if (dz >= 0) {
			tz_min = (z0 - oz) / dz;
			tz_max = (z1 - oz) / dz;
		}
		else {
			tz_min = (z1 - oz) / dz;
			tz_max = (z0 - oz) / dz;
		}
	}
	double t0, t1;
	//光线进入平面处（最靠近的平面）的最大t值 
	t0 = std::max(tz_min, std::max(tx_min, ty_min));
	//光线离开平面处（最远离的平面）的最小t值
	t1 = std::min(tz_max, std::min(tx_max, ty_max));
	return t0<t1;
}

/////////////////////////////////////////////////////////////////////
// Double3Bezier
/**
双三次贝塞尔曲面求交（四分法）
【算法流程】维护一个曲面队列，每次迭代，弹出队首的曲面，并使用patchSplit算法将其四分，对得到的4块子曲面进行包围盒求交判断，将包围盒有交点的子曲面入队。重复上述过程，直到队列为空或是队首曲面的尺寸足够小为止。对队列中所有剩余的曲面应用牛顿迭代法求交，距离最近的一个交点即为所求。
【算法性能】该算法不仅效率较高，且数值稳定，较为完美地解决了朴素牛顿迭代法存在的低效、不收敛等问题。
*/

// 队列中的节点类
struct Node
{

	EVec3d P[16];  // 子曲面的控制点
	BoundingBox aabb;	// 子曲面的AABB包围盒
						/**
						子曲面的u, v与父曲面满足一定的线性关系：
						Uparent = kU * Uchild + bU;
						Vparent = kV * Vchild + bV;
						用于从四分法分割得到的子曲面的(u, v)还原得到原曲面的(u, v)，原理与证明详见实验报告
						*/
	double kU, bU, kV, bV;
	Node(const EVec3d *P_, double kU_, double bU_, double kV_, double bV_)
		: kU(kU_), bU(bU_), kV(kV_), bV(bV_), aabb(P_) {
		for (int i = 0; i < 16; i++) P[i] = P_[i];
	}
};

using namespace std;
ofstream ofs("debug.txt");

Intersection Double3Bezier::intersect(const Vec3 &ori, const Vec3 &dir, double &maxDist,/*output*/ Vec3 *P_, Vec3 *N, Vec3 *objectColor) const
{
	// eps: 最终队列中剩余的小曲面包围盒尺寸
	const double eps = 1e-2;
	EVec3d origin(ori[0], ori[1], ori[2]);
	EVec3d direction(dir[0], dir[1], dir[2]);
	direction.normalize();

	// 子曲面队列、子曲面控制点数组
	std::queue<Node> q;
	EVec3d Pchildren[64];
	Node root(P, 1, 0, 1, 0); q.push(root);
	while (!q.empty())
	{
		// 终止条件：队列为空或者队列中的曲面已足够小
		Node parentPatch = q.front();
		if (parentPatch.aabb.infNorm() < eps) break;
		q.pop();

		// 分裂出4块小曲面，并根据父节点的k, b计算得到子节点的k, b
		patchSplit(parentPatch.P, Pchildren);
		double kU = parentPatch.kU, bU = parentPatch.bU;
		double kV = parentPatch.kV, bV = parentPatch.bV;

		// 将包围盒有交点的子节点入队（子节点k, b的算法详见报告）
		// LL：左上；LR：右上；RL：左下；RR：右下
		Node LL(Pchildren, kU / 2, bU, kV / 2, bV);
		if (LL.aabb.intersect(origin, direction))
			q.push(LL);

		Node LR(Pchildren + 16, kU / 2, bU + kU / 2, kV / 2, bV);
		if (LR.aabb.intersect(origin, direction))
			q.push(LR);

		Node RL(Pchildren + 32, kU / 2, bU, kV / 2, bV + kV / 2);
		if (RL.aabb.intersect(origin, direction))
			q.push(RL);

		Node RR(Pchildren + 48, kU / 2, bU + kU / 2, kV / 2, bV + kV / 2);
		if (RR.aabb.intersect(origin, direction))
			q.push(RR);
	}

	// 牛顿法迭代，对队列中的曲面逐个求交，并获取最短距离
	double minDist = INT_MAX;
	double bestDist = -1, bestU = -1, bestV = -1;
	Node bestPatch(P, 1, 0, 1, 0);
	while (!q.empty())
	{
		Node currentPatch = q.front(); q.pop();

		// 牛顿迭代法
		int nIter = 0;
		const int MAX_ITER = 50;
		EVec3d x = newton(origin, direction, currentPatch, MAX_ITER, &nIter);

		// 更新最优曲面
		if (nIter < MAX_ITER
			&& x(0) > EPSILON && x(0) < minDist
			&& x(1) >= -EPSILON && x(1) <= 1 + EPSILON
			&& x(2) >= -EPSILON && x(2) <= 1 + EPSILON) {
			minDist = x(0);
			bestDist = x(0); bestU = x(1); bestV = x(2);
			bestPatch = currentPatch;
		}
	}

	// 不相交的情况：距离过近/过远/UV越界
	if (bestDist < EPSILON || bestDist > maxDist
		|| bestU < -EPSILON || bestU > 1 + EPSILON
		|| bestV < -EPSILON || bestV > 1 + EPSILON)
		return MISS;

	// 如相交：计算法向量、UV坐标等
	maxDist = bestDist;
	if (P_) *P_ = ori + dir * bestDist;
	EVec3d derivU = patchDerivU(bestPatch.P, bestU, bestV);
	EVec3d derivV = patchDerivV(bestPatch.P, bestU, bestV);
	if (N)
	{
		EVec3d normal = derivU.cross(derivV);
		N->x = normal(0); N->y = normal(1); N->z = normal(2);
		*N = N->normalized();
	}
	if (N && dot(*N, dir) > 0) *N = -*N;

	// 计算纹理坐标
	if (objectColor) *objectColor = color;
	if (objectColor && texture != NULL)
	{
		double u = bestPatch.kU * bestU + bestPatch.bU;
		double v = bestPatch.kV * bestV + bestPatch.bV;
	//	ofs << "u = " << u << ", v = " << v << endl;
		*objectColor *= texture->colorUV(u, v);
	}
	return OUTSIDE;
}

// 牛顿迭代法
EVec3d Double3Bezier::newton(const EVec3d &ori, const EVec3d &dir, const Node &patch, int maxIter, int *nIter) const
{
	EVec3d x(0, 0, 0), prevX(-1, -1, -1);
	while ((x - prevX).lpNorm<Eigen::Infinity>() > EPSILON && *nIter < maxIter)
	{
		prevX = x;
		EVec3d L = ori + dir * prevX(0);
		EVec3d P_ = patchPoint(patch.P, prevX(1), prevX(2));
		EVec3d derivU = patchDerivU(patch.P, prevX(1), prevX(2));
		EVec3d derivV = patchDerivV(patch.P, prevX(1), prevX(2));

		// Jacobi矩阵
		Eigen::Matrix3d Jacobi;
		Jacobi(0) = dir(0), Jacobi(1) = dir(1), Jacobi(2) = dir(2);
		Jacobi(3) = -derivU(0), Jacobi(4) = -derivU(1), Jacobi(5) = -derivU(2);
		Jacobi(6) = -derivV(0), Jacobi(7) = -derivV(1), Jacobi(8) = -derivV(2);
		x = prevX - Jacobi.inverse() * (L - P_);
		(*nIter)++;
	}
	return x;
}

// 参考网络资源实现
void Double3Bezier::saveAsObj(EVec3d *P_)
{
	uint32_t divs = 8;
	std::unique_ptr<Eigen::Vector3d[]> P(new Eigen::Vector3d[(divs + 1) * (divs + 1)]);//(i/divs,j/divs)的向量值
	std::unique_ptr<uint32_t[]> nvertices(new uint32_t[divs * divs]);//第k个面片点数
	std::unique_ptr<uint32_t[]> vertices(new uint32_t[divs * divs * 4]);//第k个面片4个点编号数
	std::unique_ptr<Eigen::Vector3d[]> N(new Eigen::Vector3d[(divs + 1) * (divs + 1)]);//(i/divs,j/divs)的法向量值
	std::unique_ptr<Eigen::Vector2d[]> st(new Eigen::Vector2d[(divs + 1) * (divs + 1)]);//记录(i/divs,j/divs)

																						//每个面nvertives和vertices都是相同的
	for (uint16_t j = 0, k = 0; j < divs; ++j) {
		for (uint16_t i = 0; i < divs; ++i, ++k) {
			nvertices[k] = 4;
			vertices[k * 4 + 0] = (divs + 1) * j + i + 1;
			vertices[k * 4 + 1] = (divs + 1) * j + i + 2;
			vertices[k * 4 + 2] = (divs + 1) * (j + 1) + i + 2;
			vertices[k * 4 + 3] = (divs + 1) * (j + 1) + i + 1;
		}
	}

	// 在曲面上采样，生成网格
	for (uint16_t j = 0, k = 0; j <= divs; ++j) {
		double v = j / (double)divs;
		for (uint16_t i = 0; i <= divs; ++i, ++k) {
			double u = i / (double)divs;
			P[k] = patchPoint(P_, u, v);
			Eigen::Vector3d dU = patchDerivU(P_, u, v);
			Eigen::Vector3d dV = patchDerivV(P_, u, v);
			N[k] = dU.cross(dV);
			N[k].normalize();
			st[k](0) = u;
			st[k](1) = v;
		}
	}

	//输出obj文件
	std::ofstream fout(name + ".obj");
	for (int i = 0; i < (divs + 1) * (divs + 1); i++)
		fout << "v " << P[i][0] << " " << P[i][1] << " " << P[i][2] << std::endl;
	for (int i = 0; i < divs * divs; i++) 
		fout << "f " << vertices[4 * i + 0] << " " << vertices[4 * i + 1] << " " << vertices[4 * i + 2] << " " << vertices[4 * i + 3] << std::endl;
	fout.close();
}