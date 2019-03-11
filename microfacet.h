#ifndef MICROFACET_H
#define MICROFACET_H

#include "glm/glm.hpp"
#include "constants.h"
#include "glm/gtx/vector_angle.hpp"
#include "glm/gtx/random.hpp"

glm::vec3 w_t(glm::vec3 wp, glm::vec3 wg);
float pdot(glm::vec3 a, glm::vec3 b);
float ap_w(glm::vec3 w_, glm::vec3 wp, glm::vec3 wg);
float at_w(glm::vec3 w_, glm::vec3 wp, glm::vec3 wt, glm::vec3 wg);
float G1(glm::vec3 wo, glm::vec3 w_, glm::vec3 wg, float ap, float at);
float lambda_p(float ap, float at);
float lambda_t(float ap, float at);
float getMicrofacetBRDF(glm::vec3 wi, glm::vec3 wp, glm::vec3 wt, glm::vec3 wo, glm::vec3 wg, float BRDF);
glm::vec3 getReflectedMicrosurfaceRay(glm::vec3 wi, glm::vec3 wp, glm::vec3 wt, glm::vec3 wg, glm::vec3 *current_normal);

#endif