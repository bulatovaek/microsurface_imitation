#include "microfacet.h"

glm::vec3 RandomCosineVectorOf1(glm::vec3 normal){
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


glm::vec3 w_t(glm::vec3 wp, glm::vec3 wg) {
	glm::vec3 wt = glm::normalize(glm::cross(wp, wg));
	if (glm::angle(glm::rotate(wt, 90.f, wg), wp) < 90.f)
		wt = glm::normalize(glm::rotate(wt, -90.f, wg));
	else 
		wt = glm::normalize(glm::rotate(wt, -90.f, wg));
	return wt;
}

float pdot(glm::vec3 a, glm::vec3 b) {
	return glm::max(0.f, glm::dot(a, b));
}

float lambda_p(glm::vec3 wp, glm::vec3 wi, glm::vec3 n) {
	float i_dot_p = pdot(wp, wi);
	return i_dot_p / (i_dot_p + pdot(w_t(wp, n), wi) * glm::sin(glm::angle(wp, n)));
}

float ap_w(glm::vec3 w_, glm::vec3 wp, glm::vec3 wg)
{
	return glm::dot(w_, wp) / glm::dot(wp, wg);
}

float at_w(glm::vec3 w_, glm::vec3 wp, glm::vec3 wt, glm::vec3 wg)
{
	return glm::dot(w_, wt) * glm::sqrt(1 - glm::dot(wp, wg) * glm::dot(wp, wg)) / glm::dot(wp, wg);
}

float lambda_p(float ap, float at)
{
	return ap / (ap + at);
}

float lambda_t(float ap, float at)
{
	return at / (ap + at);
}

float G1( glm::vec3 wo, glm::vec3 w_, glm::vec3 wg, float ap, float at)
{
	return (glm::dot(wo, w_) < 0.0f ? 0.0f : 1.0f) * glm::min(1.0f, glm::dot(wo, wg) / (ap + at));
}

float getMicrofacetBRDF(glm::vec3 wi, glm::vec3 wp, glm::vec3 wt, glm::vec3 wo, glm::vec3 wg, float BRDF)
{
	float ap_wi = ap_w(wi, wp, wg);
	float at_wi = at_w(wi, wp, wt, wg); 
	float ap_wo = ap_w(wo, wp, wg);
	float at_wo = at_w(wo, wp, wt, wg);

	float lp_wi = lambda_p(ap_wi, at_wi);
	float lt_wi = lambda_t(ap_wi, at_wi);

	float G1_wowp, G1_wowt, G1_woRefwp;
	G1_wowp = G1(wo, wp, wg, ap_wo, at_wo);
	G1_wowt = G1(wo, wt, wg, ap_wo, at_wo); 

	glm::vec3 woRef = glm::normalize(wo - 2.0f * glm::dot(wo, wt) * wt);

	float ap_woRef = ap_w(woRef, wp, wg);
	float at_woRef = at_w(woRef, wp, wt, wg); 
	G1_woRefwp = G1(woRef, wp, wg, ap_woRef, at_woRef);

	float f1 = lp_wi * BRDF * glm::dot(wo, wp) * G1_wowp +
		lt_wi * BRDF * glm::dot(wo, wt) * G1_wowt;

	float cosTh_woRef_wp = glm::abs(glm::dot(wp, woRef)); 
	float BRDF_woRef_wp = (COS_POW + 2.0f) / (2 * PI) * glm::pow(cosTh_woRef_wp, (float)COS_POW);

	glm::vec3 wiRef = glm::normalize(wi - 2.0f * glm::dot(wi, wt) * wt);
	float cosTh_wiRef_wt = glm::abs(glm::dot(wt, wiRef)); 
	float BRDF_woRef_wt = (COS_POW + 2.0f) / (2 * PI) * glm::pow(cosTh_wiRef_wt, (float)COS_POW);

	float f2 = glm::abs(lp_wi * BRDF * glm::dot(wo, wp) * G1_wowp);  /* i -> p -> o */

	//if (glm::dot(wo, wt) > 0)
		f2 += glm::abs(lp_wi * BRDF_woRef_wp * glm::dot(woRef, wp) * (1 - G1_woRefwp) * G1_wowp); /*  i -> p -> t -> o */

	//if (glm::dot(wi, wt) > 0)
		f2 += glm::abs((1 - lp_wi) * BRDF_woRef_wt * glm::dot(wo, wp) * G1_wowp); /* i -> t -> p -> o */

	//return f1;
	return f2;
}

glm::vec3 getReflectedMicrosurfaceRay(glm::vec3 wi, glm::vec3 wp, glm::vec3 wt, glm::vec3 wg, glm::vec3 *current_normal)
{
	glm::vec3 wr = wi;
	glm::vec3 wm;
	float r, G1_;
	int i = 0;
	float lp_wi = lambda_p(ap_w(wi, wp, wg), at_w(wi, wp, wt, wg));

	r = glm::linearRand(0.f, 1.f);

	if (r < lp_wi) /* wp -> wt */
	{
		wr = glm::normalize(wr + 2.0f * glm::dot(-wr, wp) * wp); // on wp
		G1_ = G1(wr, wp, wg, ap_w(wr, wp, wg), at_w(wr, wp, wt, wg));
		r = glm::linearRand(0.f, 1.f);
		*current_normal = wp;
		if (G1_ < r)
		{
			wr = glm::normalize(wr + 2.0f * glm::dot(-wr, wt) * wt); // on wt
			*current_normal = wt;
		}
	}
	else /* wt -> wp */
	{
		wr = glm::normalize(wr + 2.0f * glm::dot(-wr, wt) * wt); // on wt
		G1_ = G1(wr, wt, wg, ap_w(wr, wp, wg), at_w(wr, wp, wt, wg));
		wr = glm::normalize(wr + 2.0f * glm::dot(-wr, wp) * wp); // on wp
		*current_normal = wp;
	}

	return wr;
}