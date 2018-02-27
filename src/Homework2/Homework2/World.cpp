#include "World.h"
#include "Object.h"
#include "Camera.h"
#include "renderer/renderer.h"
using namespace std;

World::World() : nObject(0), nLight(0) 
{
	renderer = new Renderer;
	camera = new Camera;
}

World::~World() 
{
	delete camera;
	for (int i = 0; i < nObject; i++) delete objects[i];
	for (int i = 0; i < nLight; i++) delete lights[i];
}

// 渲染、保存工作全部委托给渲染引擎完成
void World::render() { 
	renderer->render(this); 
}
void World::saveImg(const std::string &fileName) { 
	renderer->saveImg(fileName); 
}