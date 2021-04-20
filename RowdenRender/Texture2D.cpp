#include "Texture2D.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Texture2D::Texture2D() {
	glGenTextures(1, &texture);
}


Texture2D::Texture2D(const char * filename) {
	stbi_set_flip_vertically_on_load(true);
	unsigned char * data = loadTextureData(filename);
	if (!data) {
		std::cerr << "Failed to load texture at :" << filename << std::endl;
		return;
	}
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	GLint imageFormat;
	switch (numChannels) {
	case 1:
		imageFormat = GL_R;
		break;
	case 2:
		imageFormat = GL_RG;
		break;
	case 3:
		imageFormat = GL_RGB;
		break;
	case 4:
		imageFormat = GL_RGBA;
		break;
	default:
		return;
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, imageFormat, width, height, 0, imageFormat, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);
}

Texture2D::Texture2D(aiTexture *texture) {
	unsigned char* image_data = nullptr;
	glGenTextures(1, &this->texture);
	glBindTexture(GL_TEXTURE_2D, this->texture);

	if (texture->mHeight == 0)
	{
		image_data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(texture->pcData), texture->mWidth, &width, &height, &numChannels, 0);
	}
	else
	{
		image_data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(texture->pcData), texture->mWidth * texture->mHeight, &width, &height, &numChannels, 0);
	}

	if (numChannels == 3)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
	}
	else
		if (numChannels == 4)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
		}
}

Texture2D::Texture2D(std::vector<glm::vec4> *colors, int height, int width) {
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	this->height = height;
	this->width = width;
	numChannels = 4;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, colors->data());
	glGenerateMipmap(GL_TEXTURE_2D);
}
Texture2D::Texture2D(std::vector<unsigned char> *vals, int height, int width) {
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	this->height = height;
	this->width = width;
	numChannels = 1;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, vals->data());
	glGenerateMipmap(GL_TEXTURE_2D);
}
Texture2D::Texture2D(unsigned char *vals, int height, int width) {
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	this->height = height;
	this->width = width;
	numChannels = 1;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, vals);
	glGenerateMipmap(GL_TEXTURE_2D);
}Texture2D::Texture2D(unsigned char **vals, int height, int width) {
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	this->height = height;
	this->width = width;
	numChannels = 1;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, vals[0]);
	glGenerateMipmap(GL_TEXTURE_2D);
}Texture2D::Texture2D(unsigned short *vals, int height, int width) {
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	this->height = height;
	this->width = width;
	numChannels = 1;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16, width, height, 0, GL_RG, GL_SHORT, vals);
	glGenerateMipmap(GL_TEXTURE_2D);
}Texture2D::Texture2D(float *vals, int height, int width) {
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	this->height = height;
	this->width = width;
	numChannels = 1;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, vals);
	glGenerateMipmap(GL_TEXTURE_2D);
}

Texture2D::Texture2D(std::vector<std::string> faces)
{
	is_cube = true;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	//stbi_set_flip_vertically_on_load(true);

	
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		int width, height, nrChannels;
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 3);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

}

void Texture2D::init_from_vector(std::vector<glm::vec4> *colors, int height, int width) {
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	this->height = height;
	this->width = width;
	this->numChannels = 4;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, colors->data());
}

void Texture2D::giveName(std::string name) {
	this->name = name;
}

Texture2D::Texture2D(Texture2D::COLORS color) {
	glm::vec4 c = glm::vec4(1);
	switch (color) {
		//cases for more colors
	}
	std::vector<glm::vec4> ret;
	ret.emplace_back(c);
	init_from_vector(&ret, 1, 1);
}

Texture2D::Texture2D(glm::vec4 color) {
	std::vector<glm::vec4> *c = new std::vector<glm::vec4>();
	c->emplace_back(color);
	init_from_vector(c, 1, 1);
}


void Texture2D::setBorderColor(glm::vec4 color) {

	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &color[0]);
}

void Texture2D::updateTexture(std::vector<glm::vec4> *colors, int height, int width) {
	glBindTexture(GL_TEXTURE_2D, texture);
	this->height = height;
	this->width = width;
	numChannels = 4;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, colors->data());
	glGenerateMipmap(GL_TEXTURE_2D);
}

//Set parameter s and then t with content for wrapping
void Texture2D::setTexParameterWrap(GLint s, GLint t) {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, t);
}

void Texture2D::setTexParameterWrap(GLint wrap) {
	setTexParameterWrap(wrap, wrap);
}

void Texture2D::setTexMinMagFilter(GLint min, GLint mag) {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);
}

void Texture2D::setTexMinMagFilter(GLint filter) {
	setTexMinMagFilter(filter, filter);
}

unsigned char* Texture2D::loadTextureData(const char *filename) {
	return stbi_load(filename, &width, &height, &numChannels, 0);
}

void Texture2D::Bind() {
	if (is_cube)
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	else
		glBindTexture(GL_TEXTURE_2D, texture);
}


