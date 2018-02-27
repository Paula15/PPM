#include "Sphere.h"
using namespace std;

const Vec3 Sphere::DEFAULT_TEXU = Vec3(1, 0, -3);
const Vec3 Sphere::DEFAULT_TEXV = Vec3(0, 1, 0);

// 调用此函数前，必须保证dir是单位向量
Intersection Sphere::intersect(const Vec3 &ori, const Vec3 &dir, double &maxDist, Vec3 *P, Vec3 *N, Color *objectColor) const
{
	Vec3 L = C - ori;

	double tangent = dot(L, dir);	// 切线长度
	double det2 = R * R - (L.length2() - tangent * tangent); // 半弦长平方
	if (det2 < EPSILON) return MISS;

	double det = sqrt(det2);
	double dist1 = tangent - det, dist2 = tangent + det;
	if (dist1 > maxDist || (dist1 < EPSILON && dist2 > maxDist)) return MISS;
	if (dist2 < EPSILON) return MISS;

	maxDist = (dist1 > EPSILON) ? dist1 : dist2;
	if (P) *P = ori + dir * maxDist;
	if (N) *N = (*P - C).normalized();
	if (objectColor) *objectColor = color * (texture == NULL ? Color(1, 1, 1) : this->texColor(*P));
	return (dist1 > EPSILON) ? OUTSIDE : INSIDE;
}

// 将球面极坐标对应到纹理空间的UV坐标
Color Sphere::texColor(const Vec3 &P) const
{
	if (texture == NULL) return Color(1, 1, 1);
	Vec3 N = (P - C).normalized();

	double theta = acos(-dot(N, texV));
	double phi = acos(min(max(dot(N, texU) / sin(theta), -1.0), 1.0));
	double u = theta / PI, v = phi / (2 * PI);
	v = (dot(N, cross(texU, texV)) < 0) ? (1 - v) : v;
	return texture->colorUV(u, v);
}	