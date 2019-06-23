#ifndef PLANE_H
#define PLANE_H

#include "embree3\rtcore.h"
#include"object.h"

class Plane : public Object {
public:
	glm::vec3 position;
	glm::vec3 normal;

	Plane(glm::vec3 p, glm::vec3 n, glm::vec3 c,  Material m, Texture nm, bool ud, unsigned int g_id)
	{
		position = p;
		normal = n;
		color = c;
		material = m;
		//reflectance = ref;
		//emission = e;
		normalmap = nm;
		heightmap = Texture();
		diffusemap = Texture();
		geomID = g_id;
		useDisplacement = ud;
	}

	Plane(glm::vec3 p, glm::vec3 n, glm::vec3 c, Material m, Texture nm, Texture hm, Texture dm, bool ud, unsigned int g_id)
	{
		position = p;
		normal = n;
		color = c;
		material = m;
		normalmap = nm;
		heightmap = hm;
		diffusemap = dm;
		useDisplacement = ud;
		geomID = g_id;
	}

	
	glm::vec3 getNormalByNormalMap(glm::vec3 p, glm::vec2 *uvv, glm::vec3 oldNormal, glm::vec3 dir, glm::vec3 *new_color, unsigned int primID, float *lastDepth )
	{

		glm::vec3 point = glm::abs(position - p);

		int w = normalmap.getWidth(), h = normalmap.getHeight();

		glm::vec2 texCoord;
		if (glm::abs(point[0]) < EPS)
		{
			texCoord[0] = point[2];
			texCoord[1] = point[1];
		}
		else if (glm::abs(point[1]) < EPS)
		{
			texCoord[0] = point[0];
			texCoord[1] = point[2];
		}
		else
		{
			texCoord[0] = point[0];
			texCoord[1] = point[1];
	    }

		texCoord[0] *= (w );
		texCoord[1] *= (h );

		texCoord[0] = (int)texCoord[0] % (w );
		texCoord[1] = (int)texCoord[1] % (h );
		//std::cout << "x = " << texCoord[0] << ", y = " << texCoord[1] << "\n";

		glm::vec2 uv = glm::vec2(texCoord[0]/(float)(w ), texCoord[1]/(float)(h ));
		
		glm::vec3 tangent = glm::cross(oldNormal, glm::vec3(1, 0, 0));
		if (!glm::length(tangent))
		{
			tangent = glm::cross(oldNormal, glm::vec3(0, 1, 0));
		}
		if (!glm::length(tangent))
		{
			tangent = glm::cross(oldNormal, glm::vec3(0, 0, 1));
		}

		tangent = glm::normalize(tangent);

		glm::vec3 bitangent = glm::cross(oldNormal, tangent);

		bitangent = glm::normalize(bitangent);

		glm::mat3x3 TBN(tangent, bitangent, oldNormal);
		glm::mat3x3 transTBN = glm::transpose(TBN);

		dir = glm::normalize(p);

		if (p.x > 0.f && glm::abs(glm::dot(normal, glm::vec3(0.f, 1.f, 0.f) - 1.f)) < EPS)
			dir.x = -dir.x;

		if (glm::abs(glm::dot(normal, glm::vec3(0, 0, 1.f) - 1.f)) < EPS)
		{
			dir.x = -glm::abs(dir.x);
			dir.y = glm::abs(dir.y);
		}

		texCoord = getTextureCoordsByParallaxOcclusionMap(p, uv, oldNormal, glm::normalize(transTBN * dir), lastDepth);
		*uvv = texCoord;

		glm::vec3 inLightDir = glm::vec3(-0.0f, 1.99f, -5.5f) - p;

		glm::vec3 color = normalmap.getPixelByUV(texCoord[0], texCoord[1]);

		if(diffusemap.isLoaded())
		   *new_color = diffusemap.getPixelByUV(texCoord[0], texCoord[1]) ;

		glm::vec3 bumpVector =
			glm::normalize(glm::vec3((color[0] - 0.5f)*2.0f, (color[1] - 0.5f)*2.0f, (color[2] - 0.5f)*2.0f));


		glm::vec3 perturbedNormal = glm::normalize(TBN * bumpVector);

		return perturbedNormal;
	}


	glm::vec2 getTextureCoordsByParallaxOcclusionMap(glm::vec3 p, glm::vec2 uv, glm::vec3 normal, glm::vec3 dir, float *lastDepth)
	{

		/* get undisplaced texture coords */
		glm::vec2 texCoord = uv;

		glm::vec3 point = glm::abs(position - p);

		glm::vec3 dirInTBN = dir; //glm::normalize(TBN * dir);

		glm::vec2 position = glm::vec2(dirInTBN.x, dirInTBN.y)  * HEIGHT_SCALE; // / dirInTBN.z;

		const float minLayers = 8.0;
		const float maxLayers = 40.0;
		float numLayers = glm::mix(maxLayers, minLayers, glm::abs(glm::dot(glm::vec3(0.0, 0.0, 1.0), dirInTBN)));

		float layerDepth = 1.0 / numLayers;
		float currentLayerDepth = 0.0;
		float currLayer = 0;

		glm::vec2 deltaTexCoords =  position / (numLayers /* dirInTBN.z*/); // (1.01f);
		if (glm::abs(glm::dot(normal, glm::vec3(0.f, 1.f, 0.f) - 1.f)) < EPS)
		{
			deltaTexCoords = -deltaTexCoords;
		}

		glm::vec2 currentTexCoords = texCoord;

		float currentDepthMapValue = heightmap.getPixelByUV(currentTexCoords[0], currentTexCoords[1]).r;

		//std::cout << "---\n" << "old tex_coords: " << texCoord.x << " " << texCoord.y << "\n";

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
		*lastDepth = beforeDepth;

		float weight = afterDepth / (afterDepth - beforeDepth);
		glm::vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0f - weight);

		texCoord = finalTexCoords;

		//std::cout << " new tex_coords: " << texCoord.x << " " << texCoord.y << "\n";

		return texCoord;
	}

	float getParallaxSelfShadow(glm::vec2 inTexCoords, glm::vec3 inLightDir, float inLastDepth) {
		float shadowMultiplier = 0.;

		float alignFactor = glm::dot(glm::vec3(0.f, 0.f, 1.f), inLightDir);

		if (alignFactor > 0.f) {

			const float _minLayers = 16.;
			const float _maxLayers = 32.;
			float numLayers = glm::mix(_maxLayers, _minLayers, glm::abs(alignFactor));
			float deltaDepth = inLastDepth / numLayers;

			glm::vec2 delta_tex = HEIGHT_SCALE * glm::vec2(inLightDir.x, inLightDir.y) / (inLightDir.z * numLayers);

			int numSamplesUnderSurface = 0;

			float currentLayerDepth = inLastDepth - deltaDepth;

			glm::vec2 currentTexCoords = inTexCoords + delta_tex;

			float currentDepthValue = heightmap.getPixelByUV(currentTexCoords[0], currentTexCoords[1]).r; //depthValue(currentTexCoords);

			float stepIndex = 1.;

			while (currentLayerDepth > 0.) {
				if (currentDepthValue < currentLayerDepth) {
					numSamplesUnderSurface++;
					float currentShadowMultiplier = (currentLayerDepth - currentDepthValue)*(1. - stepIndex / numLayers);

					shadowMultiplier = glm::max(shadowMultiplier, currentShadowMultiplier);
				}
				stepIndex++;
				currentLayerDepth -= deltaDepth;
				currentTexCoords += delta_tex;
				currentDepthValue = heightmap.getPixelByUV(currentTexCoords[0], currentTexCoords[1]).r; //depthValue(currentTexCoords);
			}

			if (numSamplesUnderSurface < 1)
				shadowMultiplier = 1.;
			else
				shadowMultiplier = (1. - shadowMultiplier) ;
		}

		//printf("\n\n shadowMultiplier = %f\n\n", shadowMultiplier);
		return shadowMultiplier;
	}

};



unsigned int addPlane(RTCDevice g_device, RTCScene scene_i,
	glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 v4,
	glm::vec3 tr1, glm::vec3 tr2 )
{
	RTCGeometry mesh = rtcNewGeometry(g_device, RTC_GEOMETRY_TYPE_TRIANGLE);

	Vertex4f* vertices = (Vertex4f*)rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(Vertex4f), 4);
	vertices[0].x = v1.x; vertices[0].y = v1.y; vertices[0].z = v1.z;
	vertices[1].x = v2.x; vertices[1].y = v2.y; vertices[1].z = v2.z;
	vertices[2].x = v3.x; vertices[2].y = v3.y; vertices[2].z = v3.z;
	vertices[3].x = v4.x; vertices[3].y = v4.y; vertices[3].z = v4.z;

	Triangle* triangles = (Triangle*)rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, sizeof(Triangle), 2);
	triangles[0].v0 = tr1.x; triangles[0].v1 = tr1.y; triangles[0].v2 = tr1.z;
	triangles[1].v0 = tr2.x; triangles[1].v1 = tr2.y; triangles[1].v2 = tr2.z;

	rtcCommitGeometry(mesh);
	unsigned int geomID = rtcAttachGeometry(scene_i, mesh);
	rtcReleaseGeometry(mesh);
	return geomID;
}


float disp_vertices[9][4] =
{
	{ 2.0f, 2.0f,-7.0f,  0.0f }, /* right, up 0 */
	{ 2.0f,  0.5f, -7.0f, 0.0f }, /* right, middle 1 */
	{ 2.0f, 0.5f,  0.0f, 0.0f }, /* middle, middle 2 */
	{ 2.0f, 2.0f, 0.0f, 0.0f }, /* middle, up 3 */
	{ 2.0f, 0.5f, 7.0f, 0.0f }, /* left, middle 4 */
	{ 2.0f, 2.0f, 7.0f, 0.0f }, /* left, up 5 */
	{ 2.0f, -1.0f, 0.0f, 0.0f }, /* middle, down 6 */
	{ 2.0f, -1.0f, -7.0f, 0.0f }, /* right, down 7 */
	{ 2.0f, -1.0f, 7.0f, 0.0f }  /* left, down 8 */
};


unsigned int disp_indices[16] = {
	0, 3, 2, 1,
	3, 5, 4, 2,
	1, 2, 6, 7,
	2, 4, 8, 6
};


unsigned int  faces[4] = {
	4, 4, 4, 4
};


Vertex4f vert_plane[4] = {
	{ 4.f, 4.f, -7.f, 0.f  },
	{ 4.f, -2.f, -7.f, 0.f },
	{ -4.f, -2.f, -7.f, 0.f},
	{ -4.f, 4.f, -7.f, 0.f }
};

unsigned int ind_plane[6] = {
	0, 1, 2,
	2, 3, 0
};

unsigned int face_plane[2] = {
	3, 3
};


#define NUM_INDICES 6
#define NUM_FACES 2
#define FACE_SIZE 3
#define EDGE_LEVEL 512.0f

unsigned int addPlaneWithDisplacement(RTCDevice g_device, RTCScene scene_i,
	glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 v4,
	glm::vec3 tr1, glm::vec3 tr2)
{
	RTCGeometry mesh = rtcNewGeometry(g_device, RTC_GEOMETRY_TYPE_SUBDIVISION);

	rtcSetSharedGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, vert_plane, 0, sizeof(Vertex4f), 4);
	rtcSetSharedGeometryBuffer(mesh, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT, ind_plane, 0, sizeof(unsigned int), NUM_INDICES);
	rtcSetSharedGeometryBuffer(mesh, RTC_BUFFER_TYPE_FACE, 0, RTC_FORMAT_UINT, face_plane, 0, sizeof(unsigned int), NUM_FACES);


	float* level = (float*)rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_LEVEL, 0, RTC_FORMAT_FLOAT, sizeof(float), NUM_INDICES);
	for (size_t i = 0; i<NUM_INDICES; i++) level[i] = EDGE_LEVEL;

	rtcSetGeometryDisplacementFunction(mesh, displacementFunction);

	rtcCommitGeometry(mesh);
	unsigned int geomID = rtcAttachGeometry(scene_i, mesh);
	rtcReleaseGeometry(mesh);
	return geomID;
}

#endif // PLANE_H