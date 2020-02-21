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
	Texture2D(std::vector<std::string> faces);
	Texture2D(std::vector<glm::vec4>* color, int height, int width);
	Texture2D(std::vector<unsigned char>* vals, int height, int width);
	//Texture2D(std::vector<unsigned short>* vals, int height, int width);
	Texture2D(unsigned char* vals, int height, int width);
	Texture2D(unsigned short* vals, int height, int width);
	unsigned int getID() { return texture; }
	void init_from_vector(std::vector<glm::vec4> *color, int height, int width);
	void giveName(std::string);
	void setBorderColor(glm::vec4 color);
	void setTexParameterWrap(GLint s, GLint t);
	void setTexParameterWrap(GLint wrap);
	void setDims(int width, int height, int numChannels) { this->width = width; this->numChannels = numChannels; this->height = height; }
	void setTexMinMagFilter(GLint min, GLint mag);
	void setTexMinMagFilter(GLint filter);
	void Bind();
	void SetTextureID(unsigned int id) { texture = id; }
	std::string name = "texture_diffuse";
	bool is_cube = false;
private:
	unsigned int texture;
	int height, width, numChannels;
	unsigned char* loadTextureData(const char* filename);
};

