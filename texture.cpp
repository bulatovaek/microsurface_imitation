#include "texture.h"

Texture::Texture(const char *filename, TextureType t, bool usingNormalMap) {
	MyImage image = loadImageFromFile(filename); /* should add some check errors*/
	width = image.width;
	height = image.height;
	texture = image;
	printf("Loading texture: %s, width - %d, height - %d\n", filename, width, height);
	//SaveImageToFile("my_tex.bmp", image);
	loaded = true;
	useAsNormalMap = usingNormalMap;
	type = t;
}

glm::vec3 Texture::getPixelByXY(unsigned x, unsigned y) const {

	if (!loaded)
		return (glm::vec3(1, 1, 1));

	float r, g, b;
	r = texture.pixels[y * width + x][0];// / 255.0f;
	g = texture.pixels[y * width + x][1];// / 255.0f;
	b = texture.pixels[y * width + x][2];// / 255.0f;
	//std::cout << texture.pixels[y * width + x][0] << " " << texture.pixels[y * width + x][1] << " " << texture.pixels[y * width + x][2] << "\n";
	return glm::vec3(r, g, b);
}

glm::vec3 Texture::getPixelByUV(double u, double v) const {

	if (!loaded)
		return (glm::vec3(1, 0, 1));

	int x = (fmod(fabs(u), 1.0)) * (width - 1);
	int y = (1. - fmod(fabs(v), 1.0)) * (height - 1);
	//printf("%f, %f\n", u, v);
	double r, g, b;
	try {
		r = (double)texture.pixels[y * width + x][0];// / 255.0f;
		g = (double)texture.pixels[y * width + x][1];// / 255.0f;
		b = (double)texture.pixels[y * width + x][2];// / 255.0f;
		return glm::vec3(r, g, b);
	}
	catch (const std::out_of_range& e){
		printf("error with uv, yx: %lf, %lf - %i, %i (width, height: %i, %i) \n", u, v, x, y, width, height);
		return glm::vec3(0, 1, 0);
	}
}

bool Texture::isLoaded() const {
	return loaded;
}

glm::vec2 Texture::textureCoordForSphere(glm::vec3 point, float r){
	glm::vec2 texCoord;
	if (useAsNormalMap){
		texCoord[0] = glm::acos(point[2] / r) / PI;
		texCoord[1] = glm::acos(point[1] / (r * glm::sin(PI * texCoord[0]))) / (2 * PI);

		texCoord[0] *= texture.width;
		texCoord[1] *= texture.height;

		texCoord[0] = (int)texCoord[0] % texture.width;
		texCoord[1] = (int)texCoord[1] % texture.height;

		return texCoord;
	}
	return glm::vec2(0, 0);
}