#define _CRT_SECURE_NO_WARNINGS

/* STANDART LIBRARIES INCLUDES */
#include <vector>

/* EMBREE INCLUDES */
#include "embree3\rtcore.h"
#include "embree3\rtcore_ray.h"

/* GLM INCLIDES */
#include "glm\gtx\random.hpp"
#include "glm\gtx\vector_angle.hpp"

/* SPEC INCLUDES */
#include "image.h"
#include "imageIO.h"
#include "Ray.h"
#include "object.h"
#include "plane.h"
#include "render.h"
#include "sphere.h"

/* END OF INCLUDES */

glm::vec3 face_colors[100];
glm::vec3 vertex_colors[8];

int width = 900;
int height = 900;

int main(int argc, char* argv[])
{
	RTCDevice g_device = nullptr;
	RTCScene g_scene = nullptr;
	MyImage img = MyImage(width, height);
	std::vector<Object*> scene_objects; /* vector of scene objects */
	unsigned int id = 0;

	g_device = rtcNewDevice(nullptr);

	/* create a device */
	error_handler(nullptr, rtcGetDeviceError(g_device), nullptr);

	/* create scene */
	g_scene = rtcNewScene(g_device);

	// down plane -- 0
	id = addPlane(g_device, g_scene,
		glm::vec3(-2.f, -1.f, -7.f), glm::vec3(-2.f, -1.f, 7.f), glm::vec3(2.f, -1.f, -7.f), glm::vec3(2.f, -1.f, 7.f),
		glm::vec3(2, 1, 0), glm::vec3(2, 3, 1));
	Object *plane1 = new Plane(glm::vec3(-2.f, -1.f, -7.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.8f, 0.7f, 0.f), 1.0f, glm::vec3(0, 0, 0), DIFF, Texture(), id);
	scene_objects.push_back(plane1);

	//up -- 1
	id = addPlane(g_device, g_scene,
		glm::vec3(-2.f, 2.f, -7.f), glm::vec3(-2.f, 2.f, 7.f), glm::vec3(2.f, 2.f, -7.f), glm::vec3(2.f, 2.f, 7.f),
		glm::vec3(0, 1, 2), glm::vec3(1, 3, 2));
	Object *plane2 = new Plane(glm::vec3(-2.f, 2.f, -7.f), glm::vec3(0, -1.f, 0), glm::vec3(0.6f, 0.01f, 0.9f), 1.0f, glm::vec3(0, 0, 0), DIFF, Texture(), id);
	scene_objects.push_back(plane2);

	//left -- 2
	id = addPlane(g_device, g_scene,
		glm::vec3(-2.f, -1.f, -7.f), glm::vec3(-2.f, -1.f, 7.f), glm::vec3(-2.f, 2.f, -7.f), glm::vec3(-2.f, 2.f, 7.f),
		glm::vec3(0, 1, 2), glm::vec3(1, 3, 2));
	Object *plane3 = new Plane(glm::vec3(-2.f, -1.f, -7.f), glm::vec3(-1.f, 0, 0), glm::vec3(0.8f, 0.001f, 0.01f), 1.0f, glm::vec3(0, 0, 0), DIFF, Texture(), id);
	scene_objects.push_back(plane3);

	//right -- 3
	id = addPlane(g_device, g_scene,
		glm::vec3(2.f, -1.f, -7.f), glm::vec3(2.f, -1.f, 7.f), glm::vec3(2.f, 2.f, -7.f), glm::vec3(2.f, 2.f, 7.f),
		glm::vec3(2, 1, 0), glm::vec3(2, 3, 1));
	Object *plane4 = new Plane(glm::vec3(2.f, -1.f, -7.f), glm::vec3(1.f, 0, 0), glm::vec3(0.08f, 0.7f, 0.01f), 1.0f, glm::vec3(0, 0, 0), DIFF, Texture(), id);
	scene_objects.push_back(plane4);

	//back -- 4
	id = addPlane(g_device, g_scene,
		glm::vec3(2.f, 2.f, -7.f), glm::vec3(2.f, -1.f, -7.f), glm::vec3(-2.f, -1.f, -7.f), glm::vec3(-2.f, 2.f, -7.f),
		glm::vec3(0, 1, 2), glm::vec3(2, 3, 0)); 
	Object *plane5 = new Plane(glm::vec3(2.f, 2.f, -7.f), glm::vec3(0, 0, -1.f), glm::vec3(0.01f, 0.01f, 0.85f), 1.0f, glm::vec3(0, 0, 0), DIFF, Texture(), id);
	scene_objects.push_back(plane5);

	// -- 5
	id = addPlane(g_device, g_scene,
		glm::vec3(2.f, 2.f, 1.f), glm::vec3(2.f, -1.f, 1.f), glm::vec3(-2.f, -1.f, 1.f), glm::vec3(-2.f, 2.f, 1.f),
		glm::vec3(2, 1, 0), glm::vec3(0, 3, 2));
	Object *plane6 = new Plane(glm::vec3(2.f, 2.f, 1.f), glm::vec3(0, 0, 1.f), glm::vec3(0.5f, 0.01f, 0.95f), 1.0f, glm::vec3(0, 0, 0), DIFF, Texture(), id);
	scene_objects.push_back(plane6);

	//light -- 6
	id = addPlane(g_device, g_scene,
		glm::vec3(-0.4f, 1.99f, -5.9f), glm::vec3(-0.4f, 1.99f, -5.1f), glm::vec3(0.4f, 1.99f, -5.9f), glm::vec3(0.4f, 1.99f, -5.1f),
		glm::vec3(0, 1, 2), glm::vec3(1, 3, 2));
	Object *light = new Plane(glm::vec3(-0.4f, 1.99f, -5.9f), glm::vec3(0, -1.f, 0), glm::vec3(1, 1, 1), 1.0f, glm::vec3(25, 25, 25), EMIT, Texture(), id);
	scene_objects.push_back(light);

	// sphere -- 7
	id = createTriangulatedSphere(g_device, g_scene, glm::vec3(-1.2, -0.3, -6), 0.7f);
	Object* sphere = new Sphere(glm::vec3(-1.2, -0.3, -6), 0.7f, glm::vec3(1, 1, 1), 1.0f, glm::vec3(0.f), SPEC, Texture("normals.bmp", SPHERE, true), id);
	scene_objects.push_back(sphere);

	/* commit changes to scene */
	rtcCommitScene(g_scene);

	renderTileStandard(&img, g_scene, scene_objects);
	SaveImageToFile("result/1.bmp", img);
	int a;
	std::cin >> a;

	/* destroy scene */
	rtcReleaseScene(g_scene); g_scene = nullptr;

	/* destroy device */
	rtcReleaseDevice(g_device);
	return 0;
}