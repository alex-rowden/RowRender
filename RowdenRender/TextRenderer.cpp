#include "TextRenderer.h"
#include <windows.h>
TextRenderer::TextRenderer() {
	auto error = FT_Init_FreeType(&library);
	if (error) {
		std::cerr << "An error has occured during the initialization of FreeType: " 
			<< error << std::endl;
	}
	char win_dir_buffer[100];
	GetEnvironmentVariable("WINDIR", (char*)&win_dir_buffer,
		sizeof(win_dir_buffer));
	error = FT_New_Face(library,
		(std::string(win_dir_buffer) + "/Fonts/Arial.ttf").c_str(),
		0,
		&arial_face);
	if (error == FT_Err_Unknown_File_Format)
	{
		std::cerr << "Unknown face file format" << std::endl;
	}
	else if (error)
	{
		std::cerr << "Error while loading face: " << error << std::endl;
	}
}

void TextRenderer::SetCharacterSize(int height) {
	characterSize = height;
	auto error = FT_Set_Char_Size(
		arial_face,    /* handle to face object           */
		0,       /* char_width in 1/64th of points  */
		height * 64,   /* char_height in 1/64th of points */
		109,     /* horizontal device resolution    */
		109);   /* vertical device resolution      */
	if (error) {
		std::cerr << "Error in setting character size: " << error << std::endl;
	}
}
void  compute_string_bbox(FT_BBox* abbox, FT_Glyph* glyphs, FT_Vector*pos, unsigned int num_glyphs)
{
	FT_BBox  bbox;
	FT_BBox  glyph_bbox;


	/* initialize string bbox to "empty" values */
	bbox.xMin = bbox.yMin = 32000;
	bbox.xMax = bbox.yMax = -32000;

	/* for each glyph image, compute its bounding box, */
	/* translate it, and grow the string bbox          */
	for (int n = 0; n < num_glyphs; n++)
	{
		FT_Glyph_Get_CBox(glyphs[n], ft_glyph_bbox_pixels,
			&glyph_bbox);

		glyph_bbox.xMin += pos[n].x;
		glyph_bbox.xMax += pos[n].x;
		glyph_bbox.yMin += pos[n].y;
		glyph_bbox.yMax += pos[n].y;

		if (glyph_bbox.xMin < bbox.xMin)
			bbox.xMin = glyph_bbox.xMin;

		if (glyph_bbox.yMin < bbox.yMin)
			bbox.yMin = glyph_bbox.yMin;

		if (glyph_bbox.xMax > bbox.xMax)
			bbox.xMax = glyph_bbox.xMax;

		if (glyph_bbox.yMax > bbox.yMax)
			bbox.yMax = glyph_bbox.yMax;
	}

	/* check that we really grew the string bbox */
	if (bbox.xMin > bbox.xMax)
	{
		bbox.xMin = 0;
		bbox.yMin = 0;
		bbox.xMax = 0;
		bbox.yMax = 0;
	}

	/* return string bbox */
	*abbox = bbox;
}

void TextRenderer::makeTex(unsigned int w, unsigned int h)
{
	this->width = w;
	this->height = h;
	this->tex = new unsigned char[w * h];
	
}
void TextRenderer::paintGlyph(FT_Bitmap bitmap, unsigned int penX, unsigned int penY)
{

	FT_Int  i, j, p, q;
	FT_Int  x_max = penX + bitmap.width;
	FT_Int  y_max = penY + bitmap.rows;


	/* for simplicity, we assume that `bitmap->pixel_mode' */
	/* is `FT_PIXEL_MODE_GRAY' (i.e., not a bitmap font)   */

	for (i = penX, p = 0; i < x_max; i++, p++)
	{
		for (j = penY, q = bitmap.rows-1; j < y_max; j++, q--)
		{
			if (i < 0 || j < 0 ||
				i >= width || j >= height)
				continue;

			tex[j * width + i] |= bitmap.buffer[q * bitmap.width + p];
		}
	}
}

void TextRenderer::RenderText(glm::uvec2 pen_pos, glm::uvec2 my_target_dims,
	std::vector<std::string> text, bool reset_text) {
	float pixel_size = characterSize * 109 / 72.0f;
	int font_height = ceil(
		pixel_size *
		(arial_face->bbox.yMax - arial_face->bbox.yMin) /
		arial_face->units_per_EM
	); int font_width = ceil(
		pixel_size *
		(arial_face->bbox.xMax - arial_face->bbox.xMin) /
		arial_face->units_per_EM
	);
	
	for (int i = 0; i < text.size(); i++) {
		RenderText(glm::uvec2(0, font_height * text.size() - (i + 1) * font_height),
			glm::uvec2(2560, text.size() * font_height),
			text.at(i), (i == 0 && reset_text));
	}
}

void TextRenderer::RenderText(glm::uvec2 pen_pos, glm::uvec2 my_target_dims, std::string text, bool reset_tex) {
	if (tex && reset_tex) {
		delete[] tex;
	}
	unsigned int pen_x = pen_pos.x;   /* start at (0,0) */
	unsigned int pen_y = pen_pos.y;

	unsigned int num_glyphs = 0;
	bool use_kerning = FT_HAS_KERNING(arial_face);
	unsigned int previous = 0, glyph_index = 0;

	FT_GlyphSlot  slot = arial_face->glyph;
	FT_Glyph      glyphs[MAX_GLYPHS];   /* glyph image    */
	FT_Vector     pos[MAX_GLYPHS];   /* glyph position */

	FT_Error error;

	for (int n = 0; n < text.length(); n++)
	{
		/* convert character code to glyph index */
		glyph_index = FT_Get_Char_Index(arial_face, text.at(n));

		/* retrieve kerning distance and move pen position */
		if (use_kerning && previous && glyph_index)
		{
			FT_Vector  delta;


			FT_Get_Kerning(arial_face, previous, glyph_index,
				FT_KERNING_DEFAULT, &delta);

			pen_x += delta.x >> 6;
			pen_y += delta.y >> 6;
		}

		/* store current pen position */
		pos[num_glyphs].x = pen_x;
		pos[num_glyphs].y = pen_y;

		/* load glyph image into the slot without rendering */
		error = FT_Load_Glyph(arial_face, glyph_index, FT_LOAD_DEFAULT);
		if (error)
			continue;  /* ignore errors, jump to next glyph */

		  /* extract glyph image and store it in our table */
		error = FT_Get_Glyph(arial_face->glyph, &glyphs[num_glyphs]);
		if (error)
			continue;  /* ignore errors, jump to next glyph */

		  /* increment pen position */
		pen_x += slot->advance.x >> 6;
		pen_y += slot->advance.y >> 6;

		/* record current glyph index */
		previous = glyph_index;

		/* increment number of glyphs */
		num_glyphs++;
	}

	FT_BBox string_bbox;
	compute_string_bbox(&string_bbox, glyphs, pos, num_glyphs);

	/* compute string dimensions in integer pixels */
	unsigned int string_width = string_bbox.xMax - string_bbox.xMin;
	unsigned int string_height = string_bbox.yMax - string_bbox.yMin;

	/* compute start pen position in 26.6 Cartesian pixels */
	unsigned int start_x = ((my_target_dims.x - string_width) / 2);
	unsigned int start_y = string_height;// ((my_target_dims.y - string_height) / 2);
	if(reset_tex)
		makeTex(my_target_dims.x, my_target_dims.y);

	for (int n = 0; n < num_glyphs; n++)
	{
		FT_Glyph   image;
		FT_Vector  pen;


		image = glyphs[n];

		pen.x = start_x + pos[n].x;
		pen.y = start_y + pos[n].y;

		error = FT_Glyph_To_Bitmap(&image, FT_RENDER_MODE_NORMAL,
			&pen, 0);

		if (!error)
		{
			FT_BitmapGlyph  bit = (FT_BitmapGlyph)image;


			paintGlyph(bit->bitmap,
				pen.x + slot->bitmap_left,
				pen.y - slot->bitmap_top / 2.0f);
			//std::cout << pen.x + slot->bitmap_left <<
			//	", " << pen.y - slot->bitmap_top << ", " <<
			//	bit->bitmap.rows << ", " <<
			//	bit->bitmap.width << std::endl;
			FT_Done_Glyph(image);
		}
		else {
			//std::cout << std::hex << error << std::endl;
		}
	}

}
