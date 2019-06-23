#include "microfacet.h"

std::default_random_engine generator_1;
std::uniform_real_distribution<double> distr_1(0.0, 1.0);

double erand48_1(int X) {
	return distr_1(generator_1);
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

float getMicrofacetBRDF(glm::vec3 wi, glm::vec3 wp, glm::vec3 wt, glm::vec3 wo, glm::vec3 wg, Material *m, unsigned short *Xi /*float BRDF*/)
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

	glm::vec3 woRef = m->getReflectedRay(wo, wt, Xi);
	float ap_woRef = ap_w(woRef, wp, wg);
	float at_woRef = at_w(woRef, wp, wt, wg); 
	G1_woRefwp = G1(woRef, wp, wg, ap_woRef, at_woRef);

	float cosTheta = m->getCosTheta(wi, wp, Xi);
	float BRDF = m->getBRDF(wi, wp, cosTheta);
	float f1 = lp_wi * BRDF * glm::dot(wo, wp) * G1_wowp +
		lt_wi * BRDF * glm::dot(wo, wt) * G1_wowt;

	float cosTh_woRef_wp = m->getCosTheta(woRef, wp, Xi); 
	float BRDF_woRef_wp = m->getBRDF(woRef, wp, cosTh_woRef_wp); 

	glm::vec3 wiRef = m->getReflectedRay(wi, wg, Xi);
	float cosTh_wiRef_wt = m->getCosTheta(wiRef, wt, Xi);
	float BRDF_woRef_wt = m->getBRDF(wiRef, wt, cosTh_wiRef_wt);

	float f2 = glm::abs(lp_wi * BRDF * glm::dot(wo, wp) * G1_wowp);  /* i -> p -> o */

	if (glm::dot(wo, wt) > 0)
		f2 += glm::abs(lp_wi * BRDF_woRef_wp * glm::dot(woRef, wp) * (1 - G1_woRefwp) * G1_wowp); /*  i -> p -> t -> o */

	if (glm::dot(wi, wt) > 0)
		f2 += glm::abs((1 - lp_wi) * BRDF_woRef_wt * glm::dot(wo, wp) * G1_wowp); /* i -> t -> p -> o */

	return f2;
}

glm::vec3 getReflectedMicrosurfaceRay(glm::vec3 wi, glm::vec3 wp, glm::vec3 wt, glm::vec3 wg, glm::vec3 *current_normal)
{
	glm::vec3 wr = wi;
	glm::vec3 wm;
	float r, G1_;
	int i = 0;
	float lp_wi = lambda_p(ap_w(wi, wp, wg), at_w(wi, wp, wt, wg));

	r = erand48_1(1);  

	if (r < lp_wi) /* wp -> wt */
	{
		wr = glm::normalize(wr + 2.0f * glm::dot(-wr, wp) * wp); // on wp
		G1_ = G1(wr, wp, wg, ap_w(wr, wp, wg), at_w(wr, wp, wt, wg));
		r = erand48_1(1); 
		*current_normal = wp;
		if (G1_ < r) /* wt -> wp */
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

float getMicrofacetPDF(glm::vec3 wi, glm::vec3 wp, glm::vec3 wt, glm::vec3 wo, glm::vec3 wg, Material *m, unsigned short *Xi)
{
	float ap_wi = ap_w(wi, wp, wg);
	float at_wi = at_w(wi, wp, wt, wg);
	float lp_wi = lambda_p(ap_wi, at_wi);

	float r = erand48_1(Xi[2]);

	float PDF = 0.0f;
	if (lp_wi > 0 ) /* we are on wp */
	{
		float cosTh_wi_wp = m->getCosTheta(wi, wp, Xi);
		float pdf_wp = m->getPDF(wi, wp, cosTh_wi_wp);
		PDF += lp_wi * pdf_wp * G1(wo, wp, wg, ap_wi, at_wi);

		if (glm::dot(wo, wt) > 1e-6) /* wp -> wt */
		{
			glm::vec3 woRef = m->getReflectedRay(wo, wt, Xi);
			float cosTh_woRef_wt = m->getCosTheta(woRef, wt, Xi);
			float pdf_wt = m->getPDF(woRef, wt, cosTh_woRef_wt);
			float ap_woRef = ap_w(woRef, wp, wg);
			float at_woRef = at_w(woRef, wp, wt, wg);
			PDF += lp_wi * pdf_wt * (1.0f - G1(woRef, wt, wg, ap_woRef, at_woRef));
		}
	}

	if (lp_wi < 1.0f && glm::dot(wi, wt) > 1e-6) /*  wt -> wp */
	{
		glm::vec3 wiRef = m->getReflectedRay(wi, wt, Xi);
		float cosTh_wiRef_wt = m->getCosTheta(wiRef, wt, Xi);
		float pdf_wt = m->getPDF(wiRef, wt, cosTh_wiRef_wt);
		PDF += (1.0f - lp_wi)* pdf_wt;
	}

	return PDF;
}