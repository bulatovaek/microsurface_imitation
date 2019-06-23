#ifndef MESH_H
#define MESH_H

#include "embree3\rtcore.h"
#include "render.h"

struct Face
{
	int vertex1_index;
	int vertex2_index;
	int vertex3_index;
	int tex_coord_index;
	int normal_index;
};

void computeTangentBasis(
	// inputs
	std::vector<glm::vec3> & vertices,
	std::vector<glm::vec2> & uvs,
	std::vector<glm::vec3> & normals,
	// outputs
	std::vector<glm::vec3> & tangents,
	std::vector<glm::vec3> & bitangents
) {
	for (int i = 0; i < vertices.size(); i += 3) {

		// Shortcuts for vertices
		glm::vec3 & v0 = vertices[i + 0];
		glm::vec3 & v1 = vertices[i + 1];
		glm::vec3 & v2 = vertices[i + 2];

		// Shortcuts for UVs
		glm::vec2 & uv0 = uvs[i + 0];
		glm::vec2 & uv1 = uvs[i + 1];
		glm::vec2 & uv2 = uvs[i + 2];

		// Edges of the triangle : postion delta
		glm::vec3 deltaPos1 = v1 - v0;
		glm::vec3 deltaPos2 = v2 - v0;

		// UV delta
		glm::vec2 deltaUV1 = uv1 - uv0;
		glm::vec2 deltaUV2 = uv2 - uv0;

		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
		glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x)*r;

		tangents.push_back(tangent);
		tangents.push_back(tangent);
		tangents.push_back(tangent);

		// Same thing for binormals
		bitangents.push_back(bitangent);
		bitangents.push_back(bitangent);
		bitangents.push_back(bitangent);
	}
}

bool loadOBJ(
	const char * path, bool isPlane,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
) {
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE * file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file !\n");
		return false;
	}

	while (1) {

		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break;

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			if (!isPlane)
			{

				float tmp_x = vertex.x;
				float tmp_z = vertex.z;

				float phi = PI / 2;
				vertex.x = tmp_x * cos(phi) - tmp_z * sin(phi);
				vertex.z = tmp_x * sin(phi) + tmp_z * cos(phi);

				vertex.x = (vertex.x) * 0.7f - 1.2f;
				vertex.y = (vertex.y) * 0.7f - 0.3f;
				vertex.z = (vertex.z) * 0.7f - 5.5f;
			}
			else {
				vertex.x = (vertex.x);
				vertex.y = (vertex.y);
				vertex.z = (vertex.z) - 7.0f;
			}
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned long int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%ld/%ld/%ld %ld/%ld/%ld %ld/%ld/%ld\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else {
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each triangle
	for (unsigned int v = 0; v<vertexIndices.size(); v += 3) {
		// For each vertex of the triangle
		for (unsigned int i = 0; i<3; i += 1) {

			unsigned int vertexIndex = vertexIndices[v + i];
			glm::vec3 vertex = temp_vertices[vertexIndex - 1];

			unsigned int uvIndex = uvIndices[v + i];
			glm::vec2 uv = temp_uvs[uvIndex - 1];

			unsigned int normalIndex = normalIndices[v + i];
			glm::vec3 normal = temp_normals[normalIndex - 1];

			out_vertices.push_back(vertex);
			out_uvs.push_back(uv);
			out_normals.push_back(normal);
		}
	}

	return true;
}

class Mesh : public Object {
public:
	std::vector< glm::vec3 > vertices;
	std::vector< glm::vec2 > uvs;
	std::vector< glm::vec3 > normals;

	std::vector<glm::vec3> tangents;
	std::vector<glm::vec3> bitangents;
	glm::vec3 position;

	Mesh(const char * path, glm::vec3 c, Material m, Texture nm, bool isPlane)
	{
		loadOBJ(path, isPlane, vertices, uvs, normals);
		//computeTangentBasis(vertices, uvs, normals, tangents, bitangents);

		printf("vertices size %d\n", vertices.size());
		printf("uvs size %d\n", uvs.size());
		printf("tangent size %d\n", tangents.size());

		color = c;
		material = m;
		//reflectance = ref;
		//emission = e;
		normalmap = nm; // Texture();
		heightmap = Texture();
		diffusemap = nm;//Texture();
		//geomID = g_id;
		position = glm::vec3(-2.f, -1.f, -7.f);
		useDisplacement = false;
	}
	glm::vec3 getNormalByNormalMap(glm::vec3 p, glm::vec2 *uvv, glm::vec3 oldNormal, glm::vec3 dir, glm::vec3 *new_color, unsigned int primID, float *last_depth)
	{
		glm::vec3 tan = tangents[3 * primID];
		glm::vec3 bitan = bitangents[3 * primID];

		glm::vec2 uv = glm::vec2(0.0f);

		glm::vec2 texCoord = uvs[3 * primID] *uv.x + uvs[3 * primID + 1] * uv.y + uvs[3 * primID + 2] * (1 - uv.x - uv.y); //uvs[3 * primID];


		glm::mat3x3 TBN(tan, bitan, oldNormal);
		glm::mat3x3 transTBN = glm::transpose(TBN);

		glm::vec3 point = glm::abs(position - p);

		int w = normalmap.getWidth(), h = normalmap.getHeight();

		uv = glm::vec2(texCoord[0] / (float)(w), texCoord[1] / (float)(h));

		glm::vec3 color = normalmap.getPixelByXY(texCoord[0]* (normalmap.getHeight() - 1), texCoord[1]*(normalmap.getWidth() - 1));
		*new_color = color;
		glm::vec3 bumpVector =
			glm::normalize(glm::vec3((color[0] - 0.5f)*2.0f, (color[1] - 0.5f)*2.0f, (color[2] - 0.5f)*2.0f));


		glm::vec3 perturbedNormal = glm::normalize(TBN * bumpVector);

		return perturbedNormal;
	}

	float getParallaxSelfShadow(glm::vec2 inTexCoords, glm::vec3 inLightDir, float inLastDepth)
	{
		return 1.0f;
	}

};

unsigned int addMesh(RTCDevice g_device, RTCScene scene_i,
	Mesh *m)
{
	RTCGeometry mesh = rtcNewGeometry(g_device, RTC_GEOMETRY_TYPE_TRIANGLE);

	int m_size = m->vertices.size();
	Vertex4f* vertices = (Vertex4f*)rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(Vertex4f), 
		m_size);
	Triangle* triangles = (Triangle*)rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, sizeof(Triangle), m_size / 3);

	int j = 0;
	for (int i = 0; i < m_size; i++)
	{
		if (i % 3 == 0)
		{
			triangles[j].v0 = i + 2; triangles[j].v1 = i + 1; triangles[j].v2 = i;
			//normals[j].x = m->normals[i].x; normals[j].y = m->normals[i].y; normals[j].z = m->normals[i].z;
			j++;
		}
		vertices[i].x = m->vertices[i].x;  vertices[i].y = m->vertices[i].y; vertices[i].z = m->vertices[i].z;
		//printf("v%d: %f %f %f \n", i, vertices[i].x, vertices[i].y, vertices[i].z);
	}

	m->vertices.clear();
	rtcCommitGeometry(mesh);
	unsigned int geomID = rtcAttachGeometry(scene_i, mesh);
	rtcReleaseGeometry(mesh);
	return geomID;
}

#endif // MESH