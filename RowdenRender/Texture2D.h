#pragma once
#include "RowRender.h"
#include <string>
class Texture2D
{
public:
	Texture2D(const char *filename);
	void setBorderColor(glm::vec4 color);
	void setTexParameterWrap(GLint s, GLint t);
	void setTexParameterWrap(GLint wrap);
	void setTexMinMagFilter(GLint min, GLint mag);
	void setTexMinMagFilter(GLint filter);
	void Bind();
	std::string name = "texture_diffuse";
private:
	unsigned int texture;
	int height, width, numChannels;
	unsigned char* loadTextureData(const char* filename);
};

