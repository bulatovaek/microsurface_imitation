#ifndef DISPLACEMENT_H
#define DISPLACEMENT_H

#include "glm\glm.hpp"
#include "embree3\rtcore.h"
#include "texture.h"

void displacementFunction(const struct RTCDisplacementFunctionNArguments* args);

#endif // DISPLACEMENT_H