#pragma once
// 三维向量类Vec3

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <algorithm>

#define EPSILON 1e-6	// 考虑浮点精度误差
#define PI 3.1415926535897932384626	
#define rand01() (rand() / double(RAND_MAX))	// 线程不安全，需每次重新srand
struct Vec3;
typedef Vec3 Color;

/**
Vec3类，本程序中三维坐标、三维向量、RGB颜色(0~1)均使用此类表示
使用v.x或v[0]均可访问v的第一个分量
*/
struct Vec3
{
	double x, y, z;
	Vec3() : x(0), y(0), z(0) {}
	Vec3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}
	
	// 计算色光功率
	double power() const { return (x + y + z) / 3.0; }

	// 计算模长/模长的平方/单位化的向量
	double length() const { return sqrt(x * x + y * y + z * z); }
	double length2() const { return x * x + y * y + z * z; }
	Vec3 normalized() const;

	// 计算反射/折射之后的单位向量
	Vec3 reflected(const Vec3 &N) const;
	Vec3 refracted(const Vec3 &N, double n) const;

	// 计算旋转之后的向量/某一法向量
	Vec3 rotated(const Vec3 &axis, double angle) const;
	Vec3 normal() const;

	// 得到一个按角度均匀分布/按余弦值均匀分布于单位球的随机单位向量
	static Vec3 random();
	static Vec3 randomCosine(const Vec3 &N);

	// 运算符重载、点乘、叉乘等
	Vec3 operator - () const { return Vec3( -x, -y, -z ); }
	Vec3 operator + ( const Vec3 &v ) const { return Vec3( x + v.x, y + v.y, z + v.z ); }
	Vec3 operator - ( const Vec3 &v ) const { return Vec3( x - v.x, y - v.y, z - v.z ); }
	Vec3 operator * ( const Vec3 &v ) const { return Vec3( x * v.x, y * v.y, z * v.z ); }
	Vec3 operator * ( double k ) const { return Vec3( x * k, y * k, z * k ); }
	Vec3 operator / ( double k ) const { return Vec3( x / k, y / k, z / k ); }
	Vec3 &operator += ( const Vec3 &v ) { return *this = *this + v; }
	Vec3 &operator -= ( const Vec3 &v ) { return *this = *this - v; }
	Vec3 &operator *= ( const Vec3 &v ) { return *this = *this * v; }
	Vec3 &operator *= ( double k ) { return *this = *this * k; }
	Vec3 &operator /= ( double k ) { return *this = *this / k; }
	bool operator == ( const Vec3 &v ) const { 
		return fabs( x - v.x ) < EPSILON && fabs( y - v.y ) < EPSILON && fabs( z - v.z ) < EPSILON;
	}
	double &operator[] ( int i ) {
		if ( i == 0 ) return x; if ( i == 1 ) return y; if ( i == 2 ) return z;
		return x;
	}
	const double operator[] ( int i ) const {
		if ( i == 0 ) return x; if ( i == 1 ) return y; if ( i == 2 ) return z;
		return x;
	}
	friend std::ostream &operator << ( std::ostream &os, const Vec3 &v ) {
		os << '(' << v.x << ", " << v.y << ", " << v.z << ')';
		return os;
	}
	friend double dot( const Vec3 &v1, const Vec3 &v2 ) {
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}
	friend Vec3 cross( const Vec3 &v1, const Vec3 &v2 ) {
		return Vec3( v1.y * v2.z - v1.z * v2.y , v1.z * v2.x - v1.x * v2.z , v1.x * v2.y - v1.y * v2.x );
	}
	friend Vec3 operator* ( double k, const Vec3 &v ) { return v * k; }
};