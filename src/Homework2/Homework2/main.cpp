#include "Object.h"
#include "Light.h"
#include "World.h"
#include "Camera.h"
#include "mesh/Mesh.h"
#include "renderer/Renderer.h"
#include <omp.h>
#include <cmath>
#include <iostream>
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>
using namespace Eigen;
using namespace cv;
using namespace std;

Mat img(800, 600, CV_8UC3);

Vector3d* createBezier(Matrix3d *change) {
	Vector3d* ret = new Vector3d[16];
	int coeff = 8;
	ret[0] = Vector3d(0, 0.5, 0) * coeff;
	ret[1] = Vector3d(1.62, 3, -2) * coeff;
	ret[2] = Vector3d(4.72, 2.5, -2) * coeff;
	ret[3] = Vector3d(10, 0, 0) * coeff;
	ret[4] = Vector3d(0, 0.5, 0) * coeff;
	ret[5] = Vector3d(2.44, 3, -1) * coeff;
	ret[6] = Vector3d(5.74, 2, -1) * coeff;
	ret[7] = Vector3d(10, 0, 0) * coeff;
	ret[8] = Vector3d(0, 0.5, 0) * coeff;
	ret[9] = Vector3d(3.56, 4.42, -0.5) * coeff;
	ret[10] = Vector3d(6.78, 3.06, -0.5) * coeff;
	ret[11] = Vector3d(10, 0, 0) * coeff;
	ret[12] = Vector3d(0, 0.5, 0) * coeff;
	ret[13] = Vector3d(3.76, 2, 0.5) * coeff;
	ret[14] = Vector3d(7.76, 2.24, -0.25) * coeff;
	ret[15] = Vector3d(10, 0, 0) * coeff;
	if (change)
		for (int i = 0; i < 16; i++) ret[i] = *change * ret[i];
	return ret;
}

Vector3d* createReel() {
	Vector3d* ret = new Vector3d[16];
	int coeff = 15;
	ret[0] = Vector3d(-2, 0.8, -1.5) * coeff;
	ret[4] = Vector3d(-2, 0.6, 0) * coeff;
	ret[8] = Vector3d(-2, 0, 1.5) * coeff;
	ret[12] = Vector3d(-2, 0.1, 3) * coeff;
	ret[1] = Vector3d(0, 0, -1.5) * coeff;
	ret[5] = Vector3d(0, 0, 0) * coeff;
	ret[9] = Vector3d(0, 0, 1.5) * coeff;
	ret[13] = Vector3d(0, 0, 3) * coeff;
	ret[2] = Vector3d(5, 0, -1.5) * coeff;
	ret[6] = Vector3d(5, 0, 0) * coeff;
	ret[10] = Vector3d(5, 0, 1.5) * coeff;
	ret[14] = Vector3d(5, 0, 3) * coeff;
	ret[3] = Vector3d(7, 0.3, -1.5) * coeff;
	ret[7] = Vector3d(7, 0, 0) * coeff;
	ret[11] = Vector3d(7, 0.6, 1.5) * coeff;
	ret[15] = Vector3d(7, 0.6, 3) * coeff;
	return ret;
}

Vector3d *createKnife()
{
	Vector3d *ret = new Vector3d[16];
	int coeff = 8;
	ret[0] = Vector3d(-3, 0, -0.3) * coeff;
	ret[4] = Vector3d(-4, 2, -1) * coeff;
	ret[8] = Vector3d(-4, 2, 1) * coeff;
	ret[12] = Vector3d(-3, 0, 0.3) * coeff;
	ret[1] = Vector3d(-1, -0.5, -0.2) * coeff;
	ret[5] = Vector3d(-1.5, 1, -0.5) * coeff;
	ret[9] = Vector3d(-1.5, 1, 0.5) * coeff;
	ret[13] = Vector3d(-1, -0.5, 0.2) * coeff;
	ret[2] = Vector3d(1, -0.5, -0.2) * coeff;
	ret[6] = Vector3d(0.5, 0.5, -0.5) * coeff;
	ret[10] = Vector3d(0.5, 0.5, 0.5) * coeff;
	ret[14] = Vector3d(1, -0.5, 0.2) * coeff;
	ret[3] = Vector3d(5, 0, -0.1) * coeff;
	ret[7] = Vector3d(4.6, 0.2, -0.1) * coeff;
	ret[11] = Vector3d(4.6, 0.2, 0.1) * coeff;
	ret[15] = Vector3d(5, 0, 0.1) * coeff;
	return ret;
}

Vec3 *createBeads(int coeff)
{
	Vec3 *pos = new Vec3[12];
	pos[0] = Vec3(0, 0, 1) * coeff;
	pos[1] = Vec3(0.5, 0, 0.86603) * coeff;
	pos[2] = Vec3(0.86603, 0, 0.5) * coeff;
	pos[3] = Vec3(1, 0, 0) * coeff;
	pos[4] = Vec3(0.86603, 0, -0.5) * coeff;
	pos[5] = Vec3(0.5, 0, -0.86603) * coeff;
	pos[6] = Vec3(0, 0, -1) * coeff;
	pos[7] = Vec3(-0.5, 0, -0.86603) * coeff;
	pos[8] = Vec3(-0.86603, 0, -0.5) * coeff;
	pos[9] = Vec3(-1, 0, 0) * coeff;
	pos[10] = Vec3(-0.86603, 0, 0.5) * coeff;
	pos[11] = Vec3(-0.5, 0, 0.86603) * coeff;
	return pos;
}

Vec3 *createLink()
{
	Vec3 *pos = new Vec3[8];
	int coeff = 3;
	pos[0] = Vec3(-0.56, 0, 3.26) * coeff;
	pos[1] = Vec3(-1.38, 0, 2.5) * coeff;
	pos[2] = Vec3(-1.2, 0, 1.42) * coeff;
	pos[3] = Vec3(-0.26, 0, 0.58) * coeff;
	pos[4] = Vec3(0.88, 0, 0.22) * coeff;
	pos[5] = Vec3(1.82, 0, -0.6) * coeff;
	pos[6] = Vec3(1.56, 0, -1.76) * coeff;
	pos[7] = Vec3(0.72, 0, -2.48) * coeff;
	return pos;
}

int main()
{
	// world
	World *world = new World;
	world->bgColor = (Vec3(0.25, 0.25, 0.25));

	// Textures
	Texture *marble = new Texture("marble.bmp");
	Texture *wood = new Texture("wood2.jpg");
	Texture *blue = new Texture("marble6.jpg");
	Texture *leaf = new Texture("leaf3.jpg");
	Texture *paper = new Texture("girls2.jpg");

	// Walls
	Sphere *frontWall = new Sphere(Vec3(0, 0, 1e5 + 2), 1e5, "front");
	frontWall->setMaterial(Vec3(.8, .8, .8), 1, 0, 0, 0, 1.4);
	world->add(frontWall);

	Sphere *backWall = new Sphere(Vec3(0, 0, -1e5 - 320), 1e5, "back");
	backWall->setMaterial(Vec3(.75, .75, .75), 1, 0, 0, 0, 1.4);
	world->add(backWall);

	Sphere *leftWall = new Sphere(Vec3(-1e5 - 150, 50, -50), 1e5, "left");
	leftWall->setMaterial(Vec3(.882, .537, .537), 1, 0, 0, 0, 1.4);
	world->add(leftWall);

	Sphere *rightWall = new Sphere(Vec3(1e5 + 150, 50, -50), 1e5, "right");
	rightWall->setMaterial(Vec3(.882, .882, .537), 1, 0, 0, 0, 1.4);
	world->add(rightWall);

	Sphere *topWall = new Sphere(Vec3(0, 1e5 + 302, -50), 1e5, "top");
	topWall->setMaterial(Vec3(.75, .75, .75), 1, 0, 0, 0, 1.4);
	world->add(topWall);

	Plane *bottomWall = new Plane(Vec3(0, 0, -140), Vec3(0, 1, 0), "floor");
	bottomWall->setMaterial(Vec3(.9, .9, .9), .8, 0, .2, 0, 1.4);
	bottomWall->setTexture(wood);
	world->add(bottomWall);

	// Light
	PointLight *light = new PointLight(Vec3(0, 95, -100), Vec3(1, 1, 1) * 1);
	world->add(light);

	PointLight *light2 = new PointLight(Vec3(125, 50, -200), Vec3(1, 1, 1) * 1);
	world->add(light2);

	PointLight *light3 = new PointLight(Vec3(-50, 20, -130), Vec3(1, 1, 1) * 0.8);
	world->add(light3);

	PointLight *light4 = new PointLight(Vec3(50, 50, -50), Vec3(1, 1, 1) * 0.8);
	world->add(light4);


	Double3Bezier* bezier = new Double3Bezier(Vec3(50, 0, -190), createBezier(NULL), "bezier");
	bezier->setMaterial(Vec3(.95, .95, .95), 1, 0, 0, 0, 1.4);
	bezier->setTexture(leaf);
	world->add(bezier);

	Matrix3d change;
	change(0) = cos(-PI / 6);  change(1) = 0; change(2) = sin(-PI / 6);
	change(3) = 0;			  change(4) = 1; change(5) = 0;
	change(6) = -sin(-PI / 6); change(7) = 0; change(8) = cos(-PI / 6);
	Double3Bezier *leaf2 = new Double3Bezier(Vec3(50, 4, -83), createBezier(&change), "leaf2");
	leaf2->setMaterial(Vec3(.95, .95, .95), 1, 0, 0, 0, 1.4);
	leaf2->setTexture(leaf);
	world->add(leaf2);

	Vec3 P = Vec3(-5, 0, -120);

	Double3Bezier *reel = new Double3Bezier(P, createReel(), "reel");
	reel->setMaterial(Vec3(.8, .8, .8), 1, 0, 0, 0, 1.4);
	reel->setTexture(paper);
	world->add(reel);

	Sphere *stdBead = new Sphere(Vec3(16, 4, -97), 4, "clayPurple");
	stdBead->setMaterial(Vec3(.9, .9, .9) * .999, .7, 0, 0.3, 0, 1.4);
	stdBead->setTexture(blue);

	// create beads
	Vec3 center(50, 4, -83);
	Vec3 *pos = createBeads(16);
	for (int i = 0; i < 12; i++)
	{
		Sphere *bead = new Sphere(*stdBead);
		bead->C = (pos[i] + center);
		world->add(bead);
	}

	Vec3 trans(-10, 0, -10);

	Sphere *mingzhu = new Sphere(Vec3(110, 32, -190), 32, "mingzhu");
	mingzhu->setMaterial(Vec3(1, 1, 1) * .999, 0, 0, 0, 1, 1.4);
	mingzhu->setTexture(marble);
	world->add(mingzhu);

	// knife
	Double3Bezier *knife = new Double3Bezier(Vec3(-22, 0, -180) + trans, createKnife(), "knife");
	knife->setMaterial(Vec3(.9, .9, .9), 0, 0, 1, 0, 1.4);
	world->add(knife);

	Double3Bezier *knife2 = new Double3Bezier(Vec3(-22, 0, -190) + trans, createKnife(), "knife");
	knife2->setMaterial(Vec3(.9, .9, .9), 0, 0, 1, 0, 1.4);
	world->add(knife2);

	Sphere *glass = new Sphere(Vec3(-48, 12, -180) + trans, 4, "glass");
	glass->setMaterial(Vec3(1, 1, 1) * .999, .7, 0, .3, 0, 1.4);
	glass->setTexture(blue);
	world->add(glass);

	Sphere *glass2 = new Sphere(Vec3(-48, 12, -190) + trans, 4, "glass");
	glass2->setMaterial(Vec3(1, 1, 1) * .999, .7, 0, .3, 0, 1.4);
	glass2->setTexture(blue);
	world->add(glass2);

	// create link
	Sphere *stdLink = new Sphere(Vec3(), 1.5, "link");
	stdLink->setMaterial(Vec3(.9, .9, .9) * .999, 0, 0, 1, 0, 1.4);

	Vec3 centerlink = Vec3(-45, 1.5, -170) + trans;
	Vec3 *poslink = createLink();
	for (int i = 0; i < 8; i++)
	{
		Sphere *link = new Sphere(*stdLink);
		link->C = (poslink[i] + centerlink);
		world->add(link);
	}

	Sphere *sphere = new Sphere(Vec3(-45, 4, -150) + trans, 6, "sphere");
	sphere->setMaterial(Vec3(.7, .9, .9), 0, 0, 0, 1, 1.4);
	world->add(sphere);

	Sphere *sphere2 = new Sphere(Vec3(-56, 6, -185) + trans, 6, "sphere");
	sphere2->setMaterial(Vec3(.7, .9, .9), 0, 0, 0, 1, 1.4);
	world->add(sphere2);

	// look at
	double s = 1 / 0.6;
	world->camera->lookAt(Vec3(0, 0, -200), 200, 160, s);

	// Render
	world->render();
	world->saveImg("test.jpg");

	Mat_<Vec3b> img = cv::imread("update.jpg", IMREAD_COLOR);
	for (int i = 0; i < img.rows; i++)
		for (int j = 0; j < img.cols; j++)
			img(i, j) *= 10;

	cv::imwrite("update_light.jpg", img);

	system("pause");
	return 0;
}
