#pragma once
#include "RowRender.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
class Texture2D
{
public:
	Texture2D(const char *filename);
	void setBorderColor(glm::vec4 color);
	void setTexParameterWrap(GLint s, GLint t);
	void setTexParameterWrap(GLint wrap);
	void setTexMinMagFilter(GLint min, GLint mag);
	void setTexMinMagFilter(GLint filter);
private:
	unsigned int texture;
	int height, width, numChannels;
	unsigned char* loadTextureData(const char* filename);
};

