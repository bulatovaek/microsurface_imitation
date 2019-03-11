#ifndef RAY_H
#define RAY_H

#include"glm\glm.hpp"
#include "constants.h"

class Ray{
public:
	glm::vec3 org;       //!< Ray origin + tnear
	float tnear;              //!< Start of ray segment
	glm::vec3 dir;        //!< Ray direction + tfar
	//float time;               //!< Time of this ray for motion blur.
	float tfar;               //!< End of ray segment
	unsigned int mask;        //!< used to mask out objects during traversal
	unsigned int id;          //!< ray ID
	unsigned int flags;       //!< ray flags

	glm::vec3 Ng;         //!< Not normalized geometry normal
	float u;                  //!< Barycentric u coordinate of hit
	float v;                  //!< Barycentric v coordinate of hit
	unsigned int primID;           //!< primitive ID
	unsigned int geomID;           //!< geometry ID
	unsigned int instID;           //!< instance ID

	Ray(const glm::vec3& org,
		const glm::vec3& dir,
		float tnear = ZERO,
		float tfar = INF,
		float time = ZERO,
		int mask = -1,
		unsigned int geomID = RTC_INVALID_GEOMETRY_ID,
		unsigned int primID = RTC_INVALID_GEOMETRY_ID,
		unsigned int instID = RTC_INVALID_GEOMETRY_ID)
		: org(org), dir(dir), tfar(tfar), mask(mask), primID(primID), geomID(geomID), instID(instID)  {}

	//float &tnear() { return org.w; }
};

RTCRayHit* RTCRayHit_(Ray& ray) {
	return (RTCRayHit*)&ray;
}

RTCRay* RTCRay_(Ray& ray) {
	return (RTCRay*)&ray;
}

void init_Ray(Ray &ray,
	const glm::vec3 org,
	const glm::vec3 dir,
	float tnear = ZERO,
	float tfar = INF,
	float time = ZERO,
	int mask = -1,
	unsigned int geomID = RTC_INVALID_GEOMETRY_ID,
	unsigned int primID = RTC_INVALID_GEOMETRY_ID,
	unsigned int instID = RTC_INVALID_GEOMETRY_ID)
{
	ray = Ray(org, dir, tnear, tfar, time, mask, geomID, primID, instID);
}

/* error reporting function */
void error_handler(void* userPtr, const RTCError code, const char* str)
{
	if (code == RTC_ERROR_NONE)
		return;

	printf("Embree: ");
	switch (code) {
	case RTC_ERROR_UNKNOWN: printf("RTC_ERROR_UNKNOWN"); break;
	case RTC_ERROR_INVALID_ARGUMENT: printf("RTC_ERROR_INVALID_ARGUMENT"); break;
	case RTC_ERROR_INVALID_OPERATION: printf("RTC_ERROR_INVALID_OPERATION"); break;
	case RTC_ERROR_OUT_OF_MEMORY: printf("RTC_ERROR_OUT_OF_MEMORY"); break;
	case RTC_ERROR_UNSUPPORTED_CPU: printf("RTC_ERROR_UNSUPPORTED_CPU"); break;
	case RTC_ERROR_CANCELLED: printf("RTC_ERROR_CANCELLED"); break;
	default: printf("invalid error code"); break;
	}
	if (str) {
		printf(" (");
		while (*str) putchar(*str++);
		printf(")\n");
	}
	exit(1);
}

#endif