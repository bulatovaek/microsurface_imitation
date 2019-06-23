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
#include "Mesh.h"

/* END OF INCLUDES */

glm::vec3 face_colors[100];
glm::vec3 vertex_colors[8];

int width = 960;
int height = 960;


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

	/* set error handler */
	rtcSetDeviceErrorFunction(g_device, error_handler, nullptr);

	/* create scene */
	g_scene = rtcNewScene(g_device);

	// down plane -- 0
	id = addPlane(g_device, g_scene,
		glm::vec3(-2.f, -1.f, -7.f), glm::vec3(-2.f, -1.f, 7.f), glm::vec3(2.f, -1.f, -7.f), glm::vec3(2.f, -1.f, 7.f),
		glm::vec3(2, 1, 0), glm::vec3(2, 3, 1));
	Object *plane1 = 
		new Plane(glm::vec3(-2.f, -1.f, -7.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.9f, 0.3f, 0.3f),
			Material(), 
			//Texture(), 
			Texture("textures/brick_normal1.bmp", PLANAR, true), 
			Texture("textures/brick_depth1.bmp", PLANAR, true), 
			//Texture(),
			Texture("textures/brick_diffuse1.bmp", PLANAR, true),
			//Texture(),
			false, id);
	scene_objects.push_back(plane1);

	//up plane -- 1

	id = addPlane(g_device, g_scene,
		glm::vec3(-2.f, 2.f, -7.f), glm::vec3(-2.f, 2.f, 7.f), glm::vec3(2.f, 2.f, -7.f), glm::vec3(2.f, 2.f, 7.f),
		glm::vec3(0, 1, 2), glm::vec3(1, 3, 2));
	Object *plane2 = new Plane(glm::vec3(-2.f, 2.f, -7.f), glm::vec3(0, -1.f, 0), glm::vec3(0.9f, 0.3f, 0.3f) ,
		Material(), Texture(), false, id);
	scene_objects.push_back(plane2);

	//left plane -- 2
	id = addPlane(g_device, g_scene,
		glm::vec3(-2.f, -1.f, -7.f), glm::vec3(-2.f, -1.f, 7.f), glm::vec3(-2.f, 2.f, -7.f), glm::vec3(-2.f, 2.f, 7.f),
		glm::vec3(0, 1, 2), glm::vec3(1, 3, 2));
	Object *plane3 = new Plane(glm::vec3(-2.f, -1.f, -7.f), glm::vec3(-1.f, 0, 0),  glm::vec3(0.8f, 0.001f, 0.01f),
		Material(), 
		Texture(), 
		false, id);
	scene_objects.push_back(plane3);

	//right plane -- 3
	id = addPlane(g_device, g_scene,
		glm::vec3(2.f, -1.f, -7.f), glm::vec3(2.f, -1.f, 7.f), glm::vec3(2.f, 2.f, -7.f), glm::vec3(2.f, 2.f, 7.f),
		glm::vec3(2, 1, 0), glm::vec3(2, 3, 1));

	Object *plane4 = new Plane(glm::vec3(2.f, 0.f, 0.f), glm::vec3(1.f, 0, 0), glm::vec3(0.8f, 0.001f, 0.01f),
		Material(),
		Texture(),
		false, id);
	scene_objects.push_back(plane4);

	//back plane -- 4
	
	/* high poly plane */
	
	/* Object* highpoly_plane = 
		new Mesh("obj/bricks4.obj", glm::vec3(0.9f, 0.3f, 0.3f), 
		Material(),Texture(), true);

	id = addMesh(g_device, g_scene, (Mesh*)highpoly_plane);
	scene_objects.push_back(highpoly_plane); */

	/* low poly plane */
	id = addPlane(g_device, g_scene,
		glm::vec3(2.f, 2.f, -7.f), glm::vec3(2.f, -1.f, -7.f), glm::vec3(-2.f, -1.f, -7.f), glm::vec3(-2.f, 2.f, -7.f),
		glm::vec3(0, 1, 2), glm::vec3(2, 3, 0)); 

	Object *plane5 = new Plane(glm::vec3(-2.f, -1.f, -7.f), glm::vec3(0, 0, -1.f), glm::vec3(0.9f, 0.3f, 0.3f),
		Material(),
		//Texture(),
		Texture("textures/brick_normal2.bmp", PLANAR, true), 
		//Texture(),
		Texture("textures/brick_depth2.bmp", PLANAR, true),
		Texture("textures/brick_diffuse2.bmp", PLANAR, true),
		//Texture(),
		false,
		id);

	scene_objects.push_back(plane5);
	
	/* displacement mapping plane */

	/*id = addPlaneWithDisplacement(g_device, g_scene,
		glm::vec3(2.f, 2.f, -7.f), glm::vec3(2.f, -1.f, -7.f), glm::vec3(-2.f, -1.f, -7.f), glm::vec3(-2.f, 2.f, -7.f),
		glm::vec3(0, 1, 2), glm::vec3(2, 3, 0));

	Object *displ_plane = new Plane(glm::vec3(-2.f, -1.f, -7.f), glm::vec3(0, 0, -1.f), glm::vec3(0.9f, 0.3f, 0.3f),
		Material(),
		Texture(), Texture(), Texture(),
		false,
		id);

		scene_objects.push_back(displ_plane);
		*/

	// plane -- 5
	id = addPlane(g_device, g_scene,
		glm::vec3(2.f, 2.f, 1.f), glm::vec3(2.f, -1.f, 1.f), glm::vec3(-2.f, -1.f, 1.f), glm::vec3(-2.f, 2.f, 1.f),
		glm::vec3(2, 1, 0), glm::vec3(0, 3, 2));
	Object *plane6 = new Plane(glm::vec3(2.f, 2.f, 1.f), glm::vec3(0, 0, 1.f), glm::vec3(0.5f, 0.01f, 0.95f), 
		 Material(), Texture(), false, id);
	scene_objects.push_back(plane6);

	//light -- 6
	id = addPlane(g_device, g_scene,
		glm::vec3(-0.4f, 1.99f, -5.9f), glm::vec3(-0.4f, 1.99f, -5.1f), glm::vec3(0.4f, 1.99f, -5.9f), glm::vec3(0.4f, 1.99f, -5.1f),
		glm::vec3(0, 1, 2), glm::vec3(1, 3, 2));
	Object *light = new Plane(glm::vec3(-0.4f, 1.99f, -5.9f), glm::vec3(0, -1.f, 0), glm::vec3(1, 1, 1),
		Material(glm::vec3(8, 8, 8), true), Texture(), false, id);
	scene_objects.push_back(light);

	// sphere -- 7

	/* triangulated low poly sphere */
	id = createTriangulatedSphere(g_device, g_scene, glm::vec3(-1.2, -0.3, -5.5), 0.7f);

	Object* sphere = new Sphere(glm::vec3(-1.2, -0.3, -5.5), 0.7f, glm::vec3(1, 1, 1), 
		 Material(0.0f, 1.0f, 1.0f, glm::vec3(0.0f), false, PHONG, false), //Texture(), Texture(),
		Texture("texture/normals.bmp", SPHERE, true), Texture(),
		id);

	scene_objects.push_back(sphere);

	/* high poly sphere */

	/*Object* highpoly_sphere = 
		new Mesh("obj/complex_sphere4.obj", glm::vec3(1.f, 1.f, 1.f), Material(0.0f, 1.0f, 1.0f, glm::vec3(0.0f), false, PHONG, false), 
		Texture( ), false);

	id = addMesh(g_device, g_scene, (Mesh*)highpoly_sphere);
	scene_objects.push_back(highpoly_sphere); */

	/* commit changes to scene */

	try {
		rtcCommitScene(g_scene);
		renderTileStandard(&img, g_scene, scene_objects);
	}
	catch (RTCError e)
	{
		printf("error!\n");
		exit(1);
	}


	SaveImageToFile("result/result.bmp", img);
	int a;
	std::cin >> a;

	/* destroy scene */
	rtcReleaseScene(g_scene); g_scene = nullptr;

	/* destroy device */
	rtcReleaseDevice(g_device);
	return 0;
}