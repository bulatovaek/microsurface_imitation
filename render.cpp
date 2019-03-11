#include "render.h"

static int count = 0;

glm::vec3 RandomCosineVectorOf(glm::vec3 normal){
	float x, y, z, cosTheta, sinTheta, phi;
	int e = COS_POW;
	glm::vec3 sphereVec, cosineVec;
	float t = 0;
	normal = glm::normalize(normal);
	do{
		sphereVec = glm::normalize(glm::sphericalRand(1.0f));
	} while (glm::abs(glm::angle(glm::normalize(sphereVec), glm::normalize(normal))) > 90.0f);
	return sphereVec;
}

/*glm::vec3 RandomCosineVectorOf(glm::vec3 normal){
	float x, y, z, cosTheta, sinTheta;
	int e = 2.0f;
	glm::vec3 sphereVec, cosineVec;
	float t = 0;
	normal = glm::normalize(normal);
	do{
		float r1 = (rand() % 100 + 1) / 100.0f;
		float r2 = (rand() % 100 + 1) / 100.0f;
		//std::cout << "r1 = " << r1 << ", r2 = "<< r2 <<"\n";
		float phi = 2 * PI * r1;
		cosTheta = glm::pow((1 - r2), 1.0f / (e + 1.0f));
		sinTheta = glm::sqrt(1 - cosTheta * cosTheta);

		x = glm::cos(phi) * sinTheta;
		y = glm::sin(phi) * sinTheta;
		z = cosTheta;
		cosineVec = glm::normalize(glm::vec3(x, y, z));
		//sphereVec = glm::normalize(glm::sphericalRand(1.0f));
		//t++;
		/*if (glm::angle(cosineVec, normal) < 90 && glm::angle(cosineVec, normal) > 0 )*/
		//std::cout << glm::angle(cosineVec, normal) << " woop\n";
	/*} while (glm::abs(glm::angle(cosineVec, normal)) >= 90.0f);
	std::cout << "woop\n";
	//return sphereVec /*sphereVec*/;
	/*return cosineVec;
} */

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
	std::cout << "MICROFACET - " << MICROFACET_ON << "\n";
	std::cout << "Start rendering\n"; 
	for (unsigned int y = 0; y< img->width; y++) for (unsigned int x = 0; x< img->height; x++)
	{
		//printf("x - %d, y - %d\n", x, y);
		/* calculate pixel color */
		glm::vec3 color = renderPixelStandard((float)x, (float)y, img->height, img->width, g_scene, scene_objects);
		float r = glm::pow(color.x, 1.0f / 2.2f);
		float g = glm::pow(color.y, 1.0f / 2.2f);
		float b = glm::pow(color.z, 1.0f / 2.2f);
		r = 255.f * glm::clamp(r, 0.0f, 1.0f);
		g = 255.f * glm::clamp(g, 0.0f, 1.0f);
		b = 255.f * glm::clamp(b, 0.0f, 1.0f);
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
glm::vec3 renderPixelStandard(float x, float y, float h, float w, RTCScene g_scene, std::vector<Object*> &scene_objects)
{
	float theta = -PI / 6;
	glm::vec3 camera_p = glm::vec3(0, 0.1, 0.1);
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

	for (int a = 0; a < SAMPLES; a++){
		color = color + traceRay(g_scene, origin_ray, 0, scene_objects, 0);
	}

	return color * (1.f / SAMPLES);
}

glm::vec3 traceRay(RTCScene g_scene, RTCRay &ray, int depth, std::vector<Object*> &scene_objects, int prev_geom_id)
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

	if (scene_objects[rayhit.hit.geomID]->type == EMIT)
	{
		glm::vec3 dir = glm::normalize( glm::vec3(ray.dir_x, ray.dir_y, ray.dir_z));
		if (glm::abs(glm::angle(dir, glm::vec3(0, -1, 0))) > 90.f)
			return scene_objects[rayhit.hit.geomID]->emission;
		else
			return glm::vec3(0, 0, 0);
	}
	else if (scene_objects[rayhit.hit.geomID]->type == DIFF)
	{
		glm::vec3 normal = -glm::normalize(glm::vec3(rayhit.hit.Ng_x, rayhit.hit.Ng_y, rayhit.hit.Ng_z));
		float tfar = rayhit.ray.tfar;
		glm::vec3 origin = glm::vec3(rayhit.ray.org_x, rayhit.ray.org_y, rayhit.ray.org_z);
		glm::vec3 direction = glm::normalize(glm::vec3(rayhit.ray.dir_x, rayhit.ray.dir_y, rayhit.ray.dir_z));
		glm::vec3 point = origin + tfar * direction;
		glm::vec3 color = scene_objects[rayhit.hit.geomID]->color;

		if (glm::abs(glm::angle(direction, normal)) < 90.f)
			return glm::vec3(0, 0, 0);


		/* Lambert BRDF */
		float BRDF_DIFF = scene_objects[rayhit.hit.geomID]->reflectance / PI;
		glm::vec3 diffuse_reflection = glm::normalize(RandomCosineVectorOf(normal) + EPS * normal);
		float cosTheta_DIFF = glm::dot(diffuse_reflection, normal);
		float PDF_DIFF = cosTheta_DIFF / PI;

		RTCRay newray;
		newray.dir_x = diffuse_reflection.x;
		newray.dir_y = diffuse_reflection.y;
		newray.dir_z = diffuse_reflection.z;
		newray.org_x = point.x;
		newray.org_y = point.y;
		newray.org_z = point.z;
		newray.tnear = ZERO + 10*EPS;
		newray.tfar = INF;
		newray.time = ZERO;
		newray.mask = -1;

		return BRDF_DIFF / PDF_DIFF * cosTheta_DIFF * color * traceRay(g_scene, newray, depth + 1, scene_objects, rayhit.hit.geomID);
	}
	else if (scene_objects[rayhit.hit.geomID]->type == SPEC)
	{
		glm::vec3 normal = -glm::normalize(glm::vec3(rayhit.hit.Ng_x, rayhit.hit.Ng_y, rayhit.hit.Ng_z));
		glm::vec3 old_normal = normal;
		float tfar = rayhit.ray.tfar;
		glm::vec3 origin = glm::vec3(rayhit.ray.org_x, rayhit.ray.org_y, rayhit.ray.org_z);
		glm::vec3 direction = glm::normalize(glm::vec3(rayhit.ray.dir_x, rayhit.ray.dir_y, rayhit.ray.dir_z));
		glm::vec3 point = origin + tfar * direction;
		glm::vec3 color = scene_objects[rayhit.hit.geomID]->color;

		if (glm::abs(glm::angle(direction, normal)) < 90.f)
			return glm::vec3(0, 0, 0);

		if (scene_objects[rayhit.hit.geomID]->texture.isLoaded() && scene_objects[rayhit.hit.geomID]->texture.isUsingNormalMap())
		{
			normal = scene_objects[rayhit.hit.geomID]->getNormalByNormalMap(point, normal);
		}

		glm::vec3 specular_reflection = glm::normalize(direction - normal * 2.0f * glm::dot(normal, direction) + 10 * EPS * normal );
		
		/* Phong BRDF */

		float cosTheta_SPEC = glm::abs(glm::dot(glm::normalize(RandomCosineVectorOf(normal)), specular_reflection));
		float BRDF_SPEC = scene_objects[rayhit.hit.geomID]->reflectance * (COS_POW + 2.0f) / (2 * PI) * glm::pow(cosTheta_SPEC, (float)COS_POW);
		float PDF_SPEC = (COS_POW + 1.0f) / (2 * PI) * glm::pow(cosTheta_SPEC, (float)COS_POW);

		float BRDF_MICROFACET = 0.0f;

		if (MICROFACET_ON){
			glm::vec3 wp = normal;
			glm::vec3 n = old_normal;
			glm::vec3 wi = direction;
			glm::vec3 wo = specular_reflection;
			glm::vec3 wt = w_t(wp, n);
			glm::vec3 current_normal;
			BRDF_MICROFACET = glm::abs(getMicrofacetBRDF(wi, wp, wt, wo, n, BRDF_SPEC));
			BRDF_SPEC = BRDF_MICROFACET;
			specular_reflection = getReflectedMicrosurfaceRay(wi, wp, wt, n, &current_normal);
			PDF_SPEC = 1.f;
			cosTheta_SPEC = 1.f;
		} 

		RTCRay newray;
		newray.dir_x = specular_reflection.x;
		newray.dir_y = specular_reflection.y;
		newray.dir_z = specular_reflection.z;
		newray.org_x = point.x;
		newray.org_y = point.y;
		newray.org_z = point.z;
		newray.tnear = ZERO + 10*EPS;
		newray.tfar = INF;
		newray.time = ZERO;
		newray.mask = -1;

		return BRDF_SPEC / PDF_SPEC * cosTheta_SPEC * color * traceRay(g_scene, newray, depth + 1, scene_objects, rayhit.hit.geomID);
	}
}
