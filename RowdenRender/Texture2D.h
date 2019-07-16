#pragma once
#include "RowRender.h"
#include <string>
#include <assimp/scene.h>
class Texture2D
{
public:
	enum class COLORS {WHITE};
	Texture2D();
	Texture2D(const char *filename);
	Texture2D(aiTexture* tex);
	Texture2D(Texture2D::COLORS color);
	Texture2D(glm::vec4 color);
	Texture2D(std::vector<glm::vec4>* color, int height, int width);
	void init_from_vector(std::vector<glm::vec4> *color, int height, int width);
	void setBorderColor(glm::vec4 color);
	void setTexParameterWrap(GLint s, GLint t);
	void setTexParameterWrap(GLint wrap);
	void setDims(int width, int height, int numChannels) { this->width = width; this->numChannels = numChannels; this->height = height; }
	void setTexMinMagFilter(GLint min, GLint mag);
	void setTexMinMagFilter(GLint filter);
	void Bind();
	std::string name = "texture_diffuse";
private:
	unsigned int texture;
	int height, width, numChannels;
	unsigned char* loadTextureData(const char* filename);
};

