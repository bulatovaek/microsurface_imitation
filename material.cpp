#include "material.h"

std::default_random_engine generator;
std::uniform_real_distribution<double> distr(0.0, 1.0);

double erand48(int X) {
	return distr(generator);
}

/*inline glm::vec3 RandomCosineVectorOf2(glm::vec3 normal) {
	float x, y, z, cosTheta, sinTheta, phi;
	int e = COS_POW;
	glm::vec3 sphereVec, cosineVec;
	float t = 0;
	normal = glm::normalize(normal);
	do {
		sphereVec = glm::normalize(glm::sphericalRand(1.0f));
	} while (glm::abs(glm::angle(glm::normalize(sphereVec), glm::normalize(normal))) > 90.0f);
	return sphereVec;
}*/

inline glm::vec3 RandomCosineVectorOf2(glm::vec3 normal, unsigned short *Xi) {
	float dx, dy, dz, cosTheta, sinTheta;
	int e = 2.0f;
	glm::vec3 sphereVec, dcosineVec, cosineVec;
	float t = 0;
	glm::vec3 nx, ny, nz, tmp, res;
	//normal = glm::normalize(normal);

	//printf("normal = (%f, %f, %f)\n", normal.x, normal.y, normal.z);
	//do {
		float r1 = (float)erand48(Xi[2]);
		float r2 = (float)erand48(Xi[2]);

		//printf("r1 = %f, r2 = %f\n", r1, r2);
		//std::cout << "r1 = " << r1 << ", r2 = "<< r2 <<"\n";
		float phi = 2 * PI * r1;
		cosTheta = glm::pow((1 - r2), 1.0f / (e + 1.0f));
		sinTheta = glm::sqrt(1 - cosTheta * cosTheta);

		dx = glm::cos(phi) * sinTheta;
		dy = glm::sin(phi) * sinTheta;
		dz = cosTheta;
		dcosineVec = glm::normalize(glm::vec3(dx, dy, dz));

		glm::vec3 tangent = glm::cross(normal, glm::vec3(0, 1, 0));
		if (!glm::length(tangent))
			tangent = glm::cross(normal, glm::vec3(0, 0, 1));
		else if (!glm::length(tangent))
			tangent = glm::cross(normal, glm::vec3(1, 0, 0));

		tangent = glm::normalize(tangent);
		glm::vec3 bitangent = glm::normalize(glm::cross(normal, tangent));
		glm::mat3x3 TBN(tangent, bitangent, normal);

		cosineVec = glm::normalize(TBN * dcosineVec);
		//printf("dcosineVec = (%f, %f, %f)\n", dcosineVec.x, dcosineVec.y, dcosineVec.z);

		//printf("cosineVec = (%f, %f, %f)\n\n", cosineVec.x, cosineVec.y, cosineVec.z);
		return cosineVec;
}

Material::Material()
{
	Kd = 1.0f;
	Ks = 0.0f;
	reflectance = 0.95f;
	emission = glm::vec3(0.0f);
	islight = false;
	brdf = LAMBERT;
	microfacet = false;
}

Material::Material(glm::vec3 e, bool l)
{
	emission = e;
	islight = l;
	microfacet = false;
}

Material::Material(float kd, float ks, float r, glm::vec3 e, bool l, BRDFType b, bool mf)
{
	Kd = kd;
	Ks = ks;
	reflectance = r;
	emission = e;
	islight = l;
	brdf = b;
	microfacet = mf;
}

glm::vec3 Material::getReflectedRay(glm::vec3 direction, glm::vec3 normal, unsigned short* Xi)
{
	float xsi = erand48(Xi[2]);// (rand() % 100 + 1) / 100.0;
	glm::vec3 ray;
	
	if (xsi <= Kd)
	{
		ray = glm::normalize(RandomCosineVectorOf2(normal, Xi) + /*+ 10 */ EPS * normal ); /* diffuse reflection */
	}
	else
	{
		ray = glm::normalize(direction - normal * 2.0f * glm::dot(normal, direction) + /*10 */ EPS * normal); /* specular reflection */
		/*float xsi1 = erand48(Xi[2]);
		float xsi2 = erand48(Xi[2]);

		float h = glm::sqrt(1.f - glm::pow(xsi1, 2.f / (COS_POW + 1)));
		glm::vec3 xyz = glm::vec3(h*glm::cos(2*PI*xsi2),  h*glm::sin(2 * PI*xsi2), glm::pow(xsi1, 1.f / (COS_POW + 1)));

		glm::vec3 tangent = glm::cross(normal, glm::vec3(0, 1, 0));
		if (!glm::length(tangent))
			tangent = glm::cross(normal, glm::vec3(0, 0, 1));
		else if (!glm::length(tangent))
			tangent = glm::cross(normal, glm::vec3(1, 0, 0));

		tangent = glm::normalize(tangent);
		glm::vec3 bitangent = glm::normalize(glm::cross(normal, tangent));
		glm::mat3x3 TBN(tangent, bitangent, normal);

		ray = glm::normalize(TBN * xyz + EPS * normal);*/
	}

	return ray;
}

float Material::getCosTheta(glm::vec3 direction, glm::vec3 normal, unsigned short* Xi)
{
	float cosTheta;
	glm::vec3 reflection;

	switch (brdf)
	{
	case LAMBERT:
		reflection = getReflectedRay(direction, normal, Xi);  //glm::normalize(RandomCosineVectorOf2(normal, Xi) +/* + 10 */ EPS*normal);
		cosTheta = glm::abs(glm::dot(reflection, normal));
		break;
	case PHONG:
		//glm::vec3 reflection_ideal = glm::normalize(direction - normal * 2.0f * glm::dot(normal, direction) + /*10 */ EPS * normal);
		reflection = getReflectedRay(direction, normal, Xi);//glm::normalize(direction - normal * 2.0f * glm::dot(normal, direction) + /*10 */ EPS * normal);
		cosTheta = glm::abs(glm::dot(glm::normalize(RandomCosineVectorOf2(normal, Xi)), reflection));
		//cosTheta = glm::abs(glm::dot(reflection_ideal, reflection));

		break;
	}

	return cosTheta;
}

float Material::getBRDF(glm::vec3 direction, glm::vec3 normal, float cosTheta)
{
	float BRDF;

	switch (brdf)
	{
	case LAMBERT:
		BRDF = reflectance / PI;
		break;

	case PHONG:
		BRDF = reflectance * (COS_POW + 2.0f) / (2 * PI) * glm::pow(cosTheta, (float)COS_POW);
		break;
	}

	return BRDF;
}

float Material::getPDF(glm::vec3 direction, glm::vec3 normal, float cosTheta)
{
	float PDF;

	switch (brdf)
	{
	case LAMBERT:
		PDF = cosTheta / PI;
		break;
	case PHONG:
		PDF = (COS_POW + 1.0f) / (2 * PI) * glm::pow(cosTheta, (float)COS_POW);
		break;
	}

	return PDF;
}