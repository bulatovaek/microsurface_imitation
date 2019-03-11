#include "string.h"
#include "atlimage.h"
#include "image.h"
#include <iostream>

void SaveImageToFile(std::string fileName, MyImage& img);
MyImage loadImageFromFile(std::string fileName);