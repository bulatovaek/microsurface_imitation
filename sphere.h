#ifndef SPHERE_H
#define SPHERE_H

#include "embree3\rtcore.h"
#include "render.h"

const int numPhi = 200;
const int numTheta = 2 * numPhi;

class Sphere : public Object {
public:
	float radius;	// Radius
	glm::vec3 position;
	Sphere(glm::vec3 p, float r, glm::vec3 c, float ref, glm::vec3 e, MaterialType t, Texture tex, unsigned int g_id)
	{
		position = p;
		radius = r;
		color = c;
		type = t;
		reflectance = ref;
		emission = e;
		texture = tex;
		geomID = g_id;
	}

	glm::vec3 getNormalByNormalMap(glm::vec3 p, glm::vec3 oldNormal){
		glm::vec2 texCoord;
		//glm::vec3 pos = glm::vec3(-1.2, -0.3, -6);
		//float radius = 0.7f;

		glm::vec3 point = position - p;
		//std::cout << "--- SPHERE I AM HERE --- \n";

		int w = texture.getWidth(), h = texture.getHeight();
		//printf("tex loaded %d \n", texture->getWidth());

		//std::cout << "point - " << point[0] << "  " << point[1] << " " << point[2] << "\n";
		texCoord[0] = glm::acos(point[1] / radius) / PI;
		texCoord[1] = glm::acos(point[0] / (radius * glm::sin(PI * texCoord[0]))) / (2 * PI);

		texCoord[0] *= w;
		texCoord[1] *= h;

		texCoord[0] = (int)texCoord[0] % w;
		texCoord[1] = (int)texCoord[1] % h;

		//std::cout << "x = " << texCoord[0] << ", y = " << texCoord[1] << "\n";


		glm::vec3 color = texture.getPixelByXY(texCoord[0], texCoord[1]);
		//std::cout << "color = " << color[0] << " " << color[1] << " " << color[2] << "\n";
		glm::vec3 bumpVector =
			glm::normalize(glm::vec3((color[0] - 0.5f)*2.0f, (color[1] - 0.5f)*2.0f, (color[2] - 0.5f)*2.0f));

		glm::vec3 localCoord = p - position;
		float theta = asin(localCoord.y / radius); //[-pi/2,pi/2]
		float phi = atan2(localCoord.z, localCoord.x); //[-pi,+pi]

		float deltaPhi = phi + 1e-5;
		glm::vec3 deltaPoint(radius * cos(theta) * cos(deltaPhi),
			radius * sin(theta),
			radius * cos(theta) * sin(deltaPhi));

		glm::vec3 tangent = glm::cross(oldNormal, glm::vec3(0, 1, 0));
		if (!glm::length(tangent))
			tangent = glm::cross(oldNormal, glm::vec3(0, 0, 1));

		tangent = glm::normalize(tangent);

		glm::vec3 bitangent = glm::cross(oldNormal, tangent);

		bitangent = glm::normalize(bitangent);
		oldNormal = glm::normalize(oldNormal);

		//std::cout << "old normal = " << oldNormal[0] << " " << oldNormal[1] << " " << oldNormal[2] << "\n";
		//std::cout << "tangent = " << tangent[0] << " " << tangent[1] << " " << tangent[2] << "\n";
		//std::cout << "bitangent = " << bitangent[0] << " " << bitangent[1] << " " << bitangent[2] << "\n";


		glm::mat3x3 TBN(tangent, bitangent, oldNormal);

		glm::vec3 perturbedNormal = glm::normalize(TBN * bumpVector);

		return perturbedNormal;

	}
};

unsigned int createTriangulatedSphere(RTCDevice g_device, RTCScene scene, const glm::vec3 p, float r)
{
	/* create triangle mesh */
	RTCGeometry geom = rtcNewGeometry(g_device, RTC_GEOMETRY_TYPE_TRIANGLE);

	/* map triangle and vertex buffers */
	Vertex4f* vertices = (Vertex4f*)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(Vertex4f), numTheta*(numPhi + 1));
	Triangle* triangles = (Triangle*)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, sizeof(Triangle), 2 * numTheta*(numPhi - 1));

	/* create sphere */
	int tri = 0;
	const float rcpNumTheta = rcp((float)numTheta);
	const float rcpNumPhi = rcp((float)numPhi);
	for (int phi = 0; phi <= numPhi; phi++)
	{
		for (int theta = 0; theta < numTheta; theta++)
		{
			const float phif = phi*float(PI)*rcpNumPhi;
			const float thetaf = theta*2.0f*float(PI)*rcpNumTheta;

			Vertex4f& v = vertices[phi*numTheta + theta];
			v.x = p.x + r*sin(phif)*sin(thetaf);
			v.y = p.y + r*cos(phif);
			v.z = p.z + r*sin(phif)*cos(thetaf);
		}
		if (phi == 0) continue;

		for (int theta = 1; theta <= numTheta; theta++)
		{
			int p00 = (phi - 1)*numTheta + theta - 1;
			int p01 = (phi - 1)*numTheta + theta%numTheta;
			int p10 = phi*numTheta + theta - 1;
			int p11 = phi*numTheta + theta%numTheta;

			if (phi > 1) {
				triangles[tri].v0 = p10;
				triangles[tri].v1 = p00;
				triangles[tri].v2 = p01;
				tri++;
			}

			if (phi < numPhi) {
				triangles[tri].v0 = p11;
				triangles[tri].v1 = p10;
				triangles[tri].v2 = p01;
				tri++;
			} 
		}
	}
	rtcCommitGeometry(geom);
	unsigned int geomID = rtcAttachGeometry(scene, geom);
	rtcReleaseGeometry(geom);
	return geomID;
}


#endif