#ifndef MATERIAL_H
#define MATERIAL_H

#include "glm\gtx\random.hpp"
#include "glm\gtx\vector_angle.hpp"
#include "constants.h"
//#include "rand48\erand48.h"
#include <random>

enum BRDFType { LAMBERT, PHONG };
//enum MaterialType { DIFF, EMIT };
class Material
{
	float Kd;
	float Ks;
	float reflectance;
	glm::vec3 emission;
	bool islight;
	BRDFType brdf;
	bool microfacet;
public:
	float getKd() { return Kd; }
	float getKs() { return Ks; }
	float getReflectance() { return reflectance; }
	glm::vec3 getEmission() { return emission; }
	bool isLight() { return islight; }
	bool useMicrofacet() { return microfacet;  }
	float getBRDF(glm::vec3 direction, glm::vec3 normal, float cosTheta);
	float getPDF(glm::vec3 direction, glm::vec3 normal, float cosTheta);
	float getCosTheta(glm::vec3 direction, glm::vec3 normal, unsigned short* Xi);
	glm::vec3 getReflectedRay(glm::vec3 direction, glm::vec3 normal, unsigned short* Xi);

	Material();
	Material(glm::vec3 e, bool l);
	Material(float kd, float ks, float r, glm::vec3 e, bool l, BRDFType b, bool mf);
};

#endif // MATERIAL_H