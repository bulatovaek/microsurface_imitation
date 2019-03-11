#include "imageIO.h"

void SaveImageToFile(std::string fileName, MyImage& img)
{
	CImage image;

	int width = img.width;
	int height = img.height;

	image.Create(width, height, 24);

	int pitch = image.GetPitch();
	unsigned char* imageBuffer = (unsigned char*)image.GetBits();

	if (pitch < 0)
	{
		imageBuffer += pitch * (height - 1);
		pitch = -pitch;
	}
	int i, j;
	int imageDisplacement = 0;
	int textureDisplacement = 0;

	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			glm::vec3 color = img.pixels[textureDisplacement + j];

			imageBuffer[imageDisplacement + j * 3] = (unsigned char)color.b; 
			imageBuffer[imageDisplacement + j * 3 + 1] = (unsigned char)color.g;
			imageBuffer[imageDisplacement + j * 3 + 2] = (unsigned char)color.r;

		}

		imageDisplacement += pitch;
		textureDisplacement += width;
	}

	image.Save(fileName.c_str());
	image.Destroy();
}

MyImage loadImageFromFile(std::string fileName){
	CImage image;
	image.Load(fileName.c_str());

	int width = image.GetWidth();
	int height = image.GetHeight();
	int i, j;
	int imageDisplacement = 0;
	int textureDisplacement = 0;
	int pitch = image.GetPitch();

	MyImage res = MyImage(width, height);

	unsigned char* imageBuffer = (unsigned char*)image.GetBits();

	if (pitch < 0)
	{
		imageBuffer += pitch * (height - 1);
		pitch = -pitch;
	}


	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			COLORREF pixel = image.GetPixel(j, i);
			glm::vec3 color = glm::vec3(GetRValue(pixel), GetGValue(pixel), GetBValue(pixel));
			res.pixels[textureDisplacement + j] = color / 255.0f;
		}
		imageDisplacement += pitch;
		textureDisplacement += width;
	}

	return res;
}