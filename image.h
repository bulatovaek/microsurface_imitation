#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include "glm/glm.hpp"

class MyImage{

public:
	int width;
	int height;
	std::vector<glm::vec3> pixels;

	MyImage(int x, int y){ width = x; height = y; pixels.resize(width * height); }

	MyImage(){ width = 0; height = 0; }
};

#endif // IMAGE_H