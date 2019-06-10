#include "Texture2D.h"

void Texture2D::setBorderColor(glm::vec4 color) {
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &color[0]);
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

