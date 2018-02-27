#include "Vec3.h"
using namespace std;

Vec3 Vec3::normalized() const
{
	double len = this->length();
	double factor = len ? 1 / len : 0;
	return *this * factor;
}

// 计算法向量为N的平面上的反射向量，调用前需单位化：v.reflected(N) = v - 2N * cos<v, N>
Vec3 Vec3::reflected(const Vec3 &N) const
{
	return *this - 2 * N * dot(*this, N);
}

// 计算法向量为N、相对折射率为n的折射向量，调用前需单位化；如果超过临界角，需考虑全反射现象
Vec3 Vec3::refracted(const Vec3 &N, double n) const
{
	double cosI = -dot(*this, N);
	double cosR2 = 1 - (1 - pow(cosI, 2)) * pow(n, 2);	
	return (cosR2 > EPSILON) ? // 是否全反射？
				*this * n + N * ( n * cosI - sqrt( cosR2 ) )
			   : this->reflected(N);
}

// 计算绕着轴axis旋转角度angle后得到的单位向量
Vec3 Vec3::rotated(const Vec3 &axis, double angle) const
{
 	if (fabs(angle) < EPSILON) return Vec3(x, y, z);
	Vec3 ret;
	double cost = cos( angle );
	double sint = sin( angle );
	ret.x += x * ( axis.x * axis.x + ( 1 - axis.x * axis.x ) * cost );
	ret.x += y * ( axis.x * axis.y * ( 1 - cost ) - axis.z * sint );
	ret.x += z * ( axis.x * axis.z * ( 1 - cost ) + axis.y * sint );
	ret.y += x * ( axis.y * axis.x * ( 1 - cost ) + axis.z * sint );
	ret.y += y * ( axis.y * axis.y + ( 1 - axis.y * axis.y ) * cost );
	ret.y += z * ( axis.y * axis.z * ( 1 - cost ) - axis.x * sint );
	ret.z += x * ( axis.z * axis.x * ( 1 - cost ) - axis.y * sint );
	ret.z += y * ( axis.z * axis.y * ( 1 - cost ) + axis.x * sint );
	ret.z += z * ( axis.z * axis.z + ( 1 - axis.z * axis.z ) * cost );
	return ret;
}

// 得到某一个法向量，常用于建立坐标系等
Vec3 Vec3::normal() const
{
	return (x == 0 && y == 0) ? 
				Vec3(1, 0, 0) : Vec3(y, -x, 0).normalized();
}

// 得到一个按照角度随机分布于单位球内的单位向量
Vec3 Vec3::random()
{
	double x, y, z;
	do {
		x = 2 * rand01() - 1;
		y = 2 * rand01() - 1;
		z = 2 * rand01() - 1;
	} while ( x * x + y * y + z * z > 1 || x * x + y * y + z * z < EPSILON );
	return Vec3( x, y, z ).normalized(); 
}

// 得到一个按照余弦值加权后随机分布于单位球内的单位向量
Vec3 Vec3::randomCosine(const Vec3 &N)
{
	// 与竖直方向的夹角theta按照余弦值加权
	double theta = acos(sqrt(rand01()));
	double phi = 2 * PI * rand01();
	Vec3 ret = N.rotated(N.normal(), theta);
    return ret.rotated(N, phi).normalized();
}