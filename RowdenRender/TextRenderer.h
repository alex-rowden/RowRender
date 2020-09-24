#pragma once
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <iostream>
#include <glm/glm.hpp>
#include <string>

#define MAX_GLYPHS 200

class TextRenderer
{
public:
	FT_Library library;
	FT_Face arial_face;
	unsigned char* tex;
	unsigned int height, width;
	TextRenderer();
	void SetCharacterSize(int);
	void makeTex(const unsigned int width, const unsigned int height);
	void paintGlyph(FT_Bitmap glyph, unsigned int penX, unsigned int penY);
	void RenderText(glm::uvec2, glm::uvec2, std::string, bool reset_tex = true);
};

