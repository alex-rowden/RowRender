#pragma once
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <iostream>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#define MAX_GLYPHS 200

class TextRenderer
{
public:
	FT_Library library;
	FT_Face arial_face;
	int characterSize;
	unsigned char* tex;
	unsigned int height, width;
	TextRenderer();
	void SetCharacterSize(int);
	void makeTex(const unsigned int width, const unsigned int height);
	void paintGlyph(FT_Bitmap glyph, unsigned int penX, unsigned int penY);
	void RenderText(glm::uvec2 pen_pos, glm::uvec2 my_target_dims, std::vector<std::string> text, bool reset_text = true);
	void RenderText(glm::uvec2, glm::uvec2, std::string, bool reset_tex = true);
};

