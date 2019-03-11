#ifndef TEXTURE_H
#define TEXTURE_H

#include "image.h"
#include "imageIO.h"
#include "constants.h"

enum TextureType { PLANAR, SPHERE };

class Texture {
private:
	unsigned width;
	unsigned height;
	bool loaded = false;
	bool useAsNormalMap = false;
	MyImage texture;
	TextureType type;
public:
	Texture(const char* filename, TextureType t, bool usingNormalMap);
	Texture(){};
	TextureType getTextureType() { return type; }
	unsigned getWidth() { return width; }
	unsigned getHeight() { return height; }
	bool isUsingNormalMap() { return useAsNormalMap; }
	glm::vec3 getPixelByXY(unsigned x, unsigned y) const;
	glm::vec3 getPixelByUV(double u, double v) const;
	glm::vec2 textureCoordForSphere(glm::vec3 point, float r);
	bool isLoaded() const;
};

#endif // TEXTURE_H