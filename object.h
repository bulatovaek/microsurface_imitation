#ifndef OBJECT_H
#define OBJECT_H

#include"glm\glm.hpp"
#include "constants.h"
#include <vector>
#include <xmmintrin.h>
#include "texture.h"

enum MaterialType { DIFF, SPEC, EMIT };

class Object {
public:
	glm::vec3 color;
	float reflectance;
	MaterialType type;
	Texture texture;
	glm::vec3 emission;
	unsigned int geomID;

	virtual glm::vec3 getNormalByNormalMap(glm::vec3 p, glm::vec3 oldNormal) = 0;
};

struct Vertex4f   { float x, y, z, r; }; 
struct Triangle { int v0, v1, v2; };

#endif // OBJECT_H