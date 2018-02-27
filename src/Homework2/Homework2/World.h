#pragma once
// 世界类World

#include "Vec3.h"
#include <vector>

class Object;
class Light;
class Camera;
class Renderer;

/**
世界类World，包括：物体、光源、相机3大要素，构成一个完整的场景，同时还需要一个渲染引擎
为效率起见，也为代码编写方便，本类并未做严格的封装
*/
class World
{
public:
	Renderer *renderer;	// 渲染引擎
	Camera *camera;	// 相机
	int nObject, nLight;	// 物体数量、光源数量
	std::vector<Object*> objects;	// 物体数组
	std::vector<Light*> lights;		// 光源数组
	Color bgColor;	// 环境颜色（背景色）

public:
	World();
	~World();
	void add(Object *object) { objects.push_back(object); }	// 添加物体
	void add(Light *light) { lights.push_back(light); }	// 添加光源
	void render();	// 渲染
	void saveImg(const std::string &fileName);	// 保存图片，支持各种格式
};