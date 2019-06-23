#ifndef OBJECT_H
#define OBJECT_H

#include"glm\glm.hpp"
#include "constants.h"
#include <vector>
#include <xmmintrin.h>
#include "texture.h"
#include "material.h"
#include "displacement.h"

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif 

enum MaterialType { DIFF, SPEC, EMIT };

class Object {
public:
	glm::vec3 color;
	//float reflectance;
	//MaterialType type;
	Material material;
	Texture normalmap;
	Texture heightmap;
	Texture diffusemap;
	//glm::vec3 emission;
	unsigned int geomID;
	bool useDisplacement;

	std::vector< glm::vec3 > vertices;
	std::vector< glm::vec2 > uvs;
	std::vector< glm::vec3 > normals;

	std::vector<glm::vec3> tangents;
	std::vector<glm::vec3> bitangents;

	virtual glm::vec3 getNormalByNormalMap(glm::vec3 p, glm::vec2 *uvv, glm::vec3 oldNormal, glm::vec3 dir, glm::vec3 *color, unsigned int primID, float *last_depth) = 0;
	virtual float getParallaxSelfShadow(glm::vec2 inTexCoords, glm::vec3 inLightDir, float inLastDepth) = 0;
};

struct Vertex4f   { float x, y, z, r; }; 
struct Normal { float x, y, z , r; };

struct Triangle { int v0, v1, v2; };

#endif // OBJECT_H