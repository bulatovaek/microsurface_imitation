#include "render.h"

float rcp(const float x)
{
	const __m128 a = _mm_set_ss(x);
	const __m128 r = _mm_rcp_ss(a);
	return _mm_cvtss_f32(_mm_mul_ss(r, _mm_sub_ss(_mm_set_ss(2.0f), _mm_mul_ss(r, a))));
}

/** Render Scene **/
void renderTileStandard(MyImage* img, RTCScene g_scene, std::vector<Object*> &scene_objects)
{
	time_t start, stop;
	time(&start);
	std::cout << "Start rendering\n"; 
#pragma omp parallel for    // OpenMP
	for ( int y = 0; y< img->width; y++) for ( int x = 0; x< img->height; x++)
	{
		/* calculate pixel color */
		unsigned short Xi[3] = { 0,0,y*y*y };
		glm::vec3 color = renderPixelStandard((float)x, (float)y, img->height, img->width, g_scene, scene_objects, Xi);
		float r = glm::clamp(color.x, 0.0f, 1.0f);
		float g = glm::clamp(color.y, 0.0f, 1.0f); 
		float b = glm::clamp(color.z, 0.0f, 1.0f);
		r = 255.f * glm::pow(r, 1.0f / 2.2f);
		g = 255.f * glm::pow(g, 1.0f / 2.2f);
		b = 255.f * glm::pow(b, 1.0f / 2.2f);
		img->pixels[y* img->width + x] = glm::vec3(r, g, b);
	}
	time(&stop);
	double diff = difftime(stop, start);
	int hrs = (int)diff / 3600;
	int mins = ((int)diff / 60) - (hrs * 60);
	int secs = (int)diff - (hrs * 3600) - (mins * 60);
	printf("\rRendering (%i samples): Complete!\nTime Taken: %i hrs, %i mins, %i secs\n\n", SAMPLES, hrs, mins, secs);
}

/** Render One Pixel **/
glm::vec3 renderPixelStandard(float x, float y, float h, float w, RTCScene g_scene, std::vector<Object*> &scene_objects, unsigned short* Xi)
{
	float theta = -PI / 6;
	glm::vec3 camera_p = glm::vec3(0, 0, 3.f);
	glm::vec3 camera_dir = glm::vec3(0, 0, 1);

	glm::vec3 direction;
	direction[0] = x + 0.5f - w / 2;
	direction[1] = y + 0.5f - h / 2;
	direction[2] = -w / glm::tan((PI / 2) / 2);
	RTCRay origin_ray;
	direction = glm::normalize(direction);

	origin_ray.dir_x = direction[0];
	origin_ray.dir_y = direction[1];
	origin_ray.dir_z = direction[2];
	origin_ray.org_x = 0;
	origin_ray.org_y = 0;
	origin_ray.org_z = 0;
	origin_ray.tnear = ZERO;
	origin_ray.tfar = INF;
	origin_ray.time = ZERO;
	origin_ray.mask = -1;

	glm::vec3 color = glm::vec3(0.0f);
	glm::vec3 current_color = glm::vec3(0.0f);

	for (int a = 0; a < SAMPLES; a++){
		//color = color + traceRay(g_scene, origin_ray, 0, scene_objects, 0, Xi);
		current_color = traceRay(g_scene, origin_ray, 0, scene_objects, 0, Xi);
		//current_color.x = glm::clamp(current_color.x, 0.0f, 1.0f);
		//current_color.y = glm::clamp(current_color.y, 0.0f, 1.0f);
		//current_color.z = glm::clamp(current_color.z, 0.0f, 1.0f);
		color = color + current_color;
	}

	return color * (1.f / SAMPLES);
}

glm::vec3 traceRay(RTCScene g_scene, RTCRay &ray, int depth, std::vector<Object*> &scene_objects, int prev_geom_id, unsigned short *Xi)
{
	if (depth == MAX_DEPTH)
	{
		return glm::vec3(0, 0, 0);
	}

	RTCRayHit rayhit;
	rayhit.ray = ray;
	rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
	rayhit.hit.primID = RTC_INVALID_GEOMETRY_ID;
	
	RTCIntersectContext context;
	rtcInitIntersectContext(&context);
	rtcIntersect1(g_scene, &context, &rayhit);

	if (rayhit.hit.geomID == RTC_INVALID_GEOMETRY_ID)
	{
		return glm::vec3(0, 0, 0);
	}


	if (scene_objects[rayhit.hit.geomID]->material.isLight())
	{
		glm::vec3 dir = glm::normalize( glm::vec3(ray.dir_x, ray.dir_y, ray.dir_z));
		if (glm::abs(glm::angle(dir, glm::vec3(0, -1, 0))) > 90.f)
			return scene_objects[rayhit.hit.geomID]->material.getEmission();
		else
			return glm::vec3(0, 0, 0);
	}

	glm::vec3 normal = -glm::normalize(glm::vec3(rayhit.hit.Ng_x, rayhit.hit.Ng_y, rayhit.hit.Ng_z));
	float tfar = rayhit.ray.tfar;
	float tnear = rayhit.ray.tnear;
	glm::vec3 origin = glm::vec3(rayhit.ray.org_x , rayhit.ray.org_y , rayhit.ray.org_z );
	glm::vec3 direction = glm::normalize(glm::vec3(rayhit.ray.dir_x, rayhit.ray.dir_y, rayhit.ray.dir_z));
	glm::vec3 point = origin + tfar * direction;
	glm::vec3 color = scene_objects[rayhit.hit.geomID]->color;
	glm::vec3 old_normal = normal;

	glm::vec3 diffuse_color;

	float last_depth;
	glm::vec2 texCoord;
	if (scene_objects[rayhit.hit.geomID]->normalmap.isLoaded() && scene_objects[rayhit.hit.geomID]->normalmap.isUsingNormalMap())
	{
		normal = 
			scene_objects[rayhit.hit.geomID]->getNormalByNormalMap(point, &texCoord, 
				normal, direction, &diffuse_color, rayhit.hit.primID, &last_depth);
	}

	if (scene_objects[rayhit.hit.geomID]->diffusemap.isLoaded())
	{
		color = diffuse_color;
	}


	float cosTheta = scene_objects[rayhit.hit.geomID]->material.getCosTheta(direction, normal, Xi);
	float PDF = scene_objects[rayhit.hit.geomID]->material.getPDF(direction, normal, cosTheta);
	float BRDF = scene_objects[rayhit.hit.geomID]->material.getBRDF(direction, normal, cosTheta);
	glm::vec3 reflection_dir = scene_objects[rayhit.hit.geomID]->material.getReflectedRay(direction, normal, Xi);

	if (PDF < EPS) PDF = EPS;

	if (scene_objects[rayhit.hit.geomID]->material.useMicrofacet()) {
		glm::vec3 wp = normal;
		glm::vec3 n = old_normal;
		glm::vec3 wi = direction;
		glm::vec3 wo = reflection_dir;
		glm::vec3 wt = w_t(wp, n);
		glm::vec3 current_normal;
		float BRDF_MICROFACET = glm::abs(getMicrofacetBRDF(wi, wp, wt, wo, n, &scene_objects[rayhit.hit.geomID]->material, Xi));
		BRDF = BRDF_MICROFACET;
		reflection_dir = glm::normalize(getReflectedMicrosurfaceRay(wi, wp, wt, n, &current_normal) + EPS * normal);
		float PDF_MICROFACET = glm::abs(getMicrofacetPDF(wi, wp, wt, wo, n, &scene_objects[rayhit.hit.geomID]->material, Xi)); 
		PDF = PDF_MICROFACET;
		if (PDF_MICROFACET < EPS) PDF =  EPS;
		cosTheta = 1.f;
	}

	if (scene_objects[rayhit.hit.geomID]->heightmap.isLoaded())
	{
		float selfShadowing = 0.0f;

		glm::vec3 tangent = glm::cross(old_normal, glm::vec3(1, 0, 0));
		if (!glm::length(tangent))
			tangent = glm::cross(old_normal, glm::vec3(0, 1, 0));
		if (!glm::length(tangent))
			tangent = glm::cross(old_normal, glm::vec3(0, 0, 1));

		tangent = glm::normalize(tangent);

		glm::vec3 bitangent = glm::cross(old_normal, tangent);

		bitangent = glm::normalize(bitangent);

		glm::mat3x3 TBN(tangent, bitangent, old_normal);
		glm::mat3x3 transTBN = glm::transpose(TBN);
		glm::vec3 inLightDir = glm::normalize(glm::vec3(-0.0f, 1.99f, -5.5f) - point); // light position
		//printf("depth %f\n", last_depth);
		float coof = 1.0f;

		selfShadowing = scene_objects[rayhit.hit.geomID]->getParallaxSelfShadow(texCoord, glm::normalize(transTBN * inLightDir), last_depth);

		color = selfShadowing * color  ;
	} 

	RTCRay newray;
	newray.dir_x = reflection_dir.x;
	newray.dir_y = reflection_dir.y;
	newray.dir_z = reflection_dir.z;
	newray.org_x = point.x;
	newray.org_y = point.y;
	newray.org_z = point.z;
	newray.tnear = ZERO + EPS;
	newray.tfar = INF;
	newray.time = ZERO;
	newray.mask = -1;

	return BRDF / PDF * cosTheta * color * traceRay(g_scene, newray, depth + 1, scene_objects, rayhit.hit.geomID, Xi);

}
