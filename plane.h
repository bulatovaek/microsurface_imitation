#ifndef PLANE_H
#define PLANE_H

#include "embree3\rtcore.h"
#include"object.h"

class Plane : public Object {
public:
	glm::vec3 position;
	glm::vec3 normal;

	Plane(glm::vec3 p, glm::vec3 n, glm::vec3 c, float ref, glm::vec3 e, MaterialType t, Texture tex, unsigned int g_id)
	{
		position = p;
		normal = n;
		color = c;
		type = t;
		reflectance = ref;
		emission = e;
		texture = tex;
		geomID = g_id;
	}
	
	glm::vec3 getNormalByNormalMap(glm::vec3 p, glm::vec3 oldNormal)
	{
		/* TO DO */
		return normal;
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

#endif // PLANE_H