#ifndef RENDER_H
#define RENDER_H

#include "image.h"
#include "embree3\rtcore.h"
#include "embree3\rtcore_ray.h"
#include "object.h"
#include <iostream>
#include "time.h"
#include "microfacet.h"
#include "glm\gtx\random.hpp"
#include "glm\gtx\vector_angle.hpp"
#include "displacement.h"
#include "rand48\erand48.h"

float rcp(const float x);
void renderTileStandard(MyImage* img, RTCScene g_scene, std::vector<Object*> &scene_objects);
glm::vec3 renderPixelStandard(float x, float y, float h, float w, RTCScene g_scene, std::vector<Object*> &scene_objects, unsigned short *Xi);
glm::vec3 traceRay(RTCScene g_scene, RTCRay &ray, int depth, std::vector<Object*> &scene_objects, int prev_geom_id, unsigned short *Xi);

#endif // RENDER_H