#ifndef SPHERE_H
#define SPHERE_H

#include "embree3\rtcore.h"
#include "render.h"

const int numPhi = 128;
const int numTheta = 2 * numPhi;

class Sphere : public Object {
public:
	float radius;	// Radius
	glm::vec3 position;
	Sphere(glm::vec3 p, float r, glm::vec3 c, Material m, Texture nm, Texture hm, Texture dm, unsigned int g_id)
	{
		position = p;
		radius = r;
		color = c;
		material = m;
		//reflectance = ref;
		//emission = e;
		normalmap = nm;
		heightmap = hm;
		diffusemap = dm;
		geomID = g_id;
		useDisplacement = false;
	}

	Sphere(glm::vec3 p, float r, glm::vec3 c, Material m, Texture nm, Texture hm, unsigned int g_id)
	{
		position = p;
		radius = r;
		color = c;
		material = m;
		//reflectance = ref;
		//emission = e;
		normalmap = nm;
		heightmap = hm;
		diffusemap = Texture();
		geomID = g_id;
		useDisplacement = false;
	}

	glm::vec3 getNormalByNormalMap(glm::vec3 p, glm::vec2 *uvv, glm::vec3 oldNormal, glm::vec3 dir, glm::vec3 *new_color, unsigned int primID, float *last_depth){
		
		glm::vec3 point = position - p;
		//std::cout << "--- SPHERE --- \n";

		int w = normalmap.getWidth(), h = normalmap.getHeight();

		//std::cout << "point - " << point[0] << "  " << point[1] << " " << point[2] << "\n";
	    glm::vec2 texCoord;
		texCoord[0] = glm::acos(point[1] / radius) / PI;
		texCoord[1] = glm::acos(point[2] / (radius * glm::sin(PI * texCoord[0]))) / (2 * PI);
		texCoord[0] *= w;
		texCoord[1] *= h;
		texCoord[0] = (int)texCoord[0] % w;
		texCoord[1] = (int)texCoord[1] % h;

		glm::vec2 uv = glm::vec2(texCoord[0] / (float)(w), texCoord[1] / (float)(h));

		glm::vec3 tangent = glm::cross(oldNormal, glm::vec3(0, 1, 0));
		if (!glm::length(tangent))
			tangent = glm::cross(oldNormal, glm::vec3(0, 0, 1));
		if (!glm::length(tangent))
			tangent = glm::cross(oldNormal, glm::vec3(1, 0, 0));

		tangent = glm::normalize(tangent);
		glm::vec3 bitangent = glm::cross(oldNormal, tangent);
		bitangent = glm::normalize(bitangent);
		oldNormal = glm::normalize(oldNormal);

		glm::mat3x3 TBN(tangent, bitangent, oldNormal);
		glm::mat3x3 transTBN = glm::transpose(TBN);

		dir = glm::normalize(p);
		texCoord = uv;
		glm::vec3 color = normalmap.getPixelByUV(texCoord[0], texCoord[1]);
		if(diffusemap.isLoaded())
		   *new_color = diffusemap.getPixelByUV(texCoord[0], texCoord[1]); 

		glm::vec3 bumpVector =
			glm::normalize(glm::vec3((color[0] - 0.5f)*2.0f, (color[1] - 0.5f)*2.0f, (color[2] - 0.5f)*2.0f));

		//std::cout << "old normal = " << oldNormal[0] << " " << oldNormal[1] << " " << oldNormal[2] << "\n";
		//std::cout << "tangent = " << tangent[0] << " " << tangent[1] << " " << tangent[2] << "\n";
		//std::cout << "bitangent = " << bitangent[0] << " " << bitangent[1] << " " << bitangent[2] << "\n";

		glm::vec3 perturbedNormal = glm::normalize(TBN * bumpVector);

		return perturbedNormal;

	}


	glm::vec2 getTextureCoordsByParallaxOcclusionMap(glm::vec3 p, glm::vec2 uv, glm::vec3 normal, glm::vec3 dir)
	{
		glm::vec2 texCoord = uv;
		glm::vec3 point = position - p;

		glm::vec3 dirInTBN = dir;

		float height_sc = 0.03f;
		glm::vec2 position = glm::vec2(dirInTBN.x, dirInTBN.y) * height_sc; // / dirInTBN.z;

		const float minLayers = 8.0;
		const float maxLayers = 32.0;
		float numLayers = glm::mix(maxLayers, minLayers, glm::abs(glm::dot(glm::vec3(0.0, 0.0, 1.0), dirInTBN)));

		float layerDepth = 1.0 / numLayers;
		float currentLayerDepth = 0.0;
		float currLayer = 0;

		glm::vec2 deltaTexCoords = position / numLayers; 
		glm::vec2 currentTexCoords = texCoord;

		float currentDepthMapValue = heightmap.getPixelByUV(currentTexCoords[0], currentTexCoords[1]).r;

		//std::cout << "---\n" << "old tex_coords: " << texCoord.x << " " << texCoord.y << "\n";
		//std::cout << " deltaTexCoords: " << deltaTexCoords.x << " " << deltaTexCoords.y << "\n";

		while (currentLayerDepth < currentDepthMapValue)
		{
			currentTexCoords -= deltaTexCoords;
			currentDepthMapValue = heightmap.getPixelByUV(currentTexCoords[0], currentTexCoords[1]).r;
			currentLayerDepth += layerDepth;
			currLayer++;
		}

		texCoord = currentTexCoords;

		glm::vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

		float afterDepth = currentDepthMapValue - currentLayerDepth;
		float beforeDepth = heightmap.getPixelByUV(prevTexCoords[0], prevTexCoords[1]).r - currentLayerDepth + layerDepth;
		float weight = afterDepth / (afterDepth - beforeDepth);

		glm::vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0f - weight);

		texCoord = finalTexCoords;

		return texCoord;
	}

	float getParallaxSelfShadow(glm::vec2 inTexCoords, glm::vec3 inLightDir, float inLastDepth) {
		return 1.0f;
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