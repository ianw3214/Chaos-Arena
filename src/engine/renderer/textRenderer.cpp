#include "textRenderer.hpp"

#include <GL/glew.h>

#include "utils/utils.hpp"

void TextRenderer::init() {
	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		ERR("Could not initialize freetype library");
	}

	// Create a font
	FT_Face face;
	if (FT_New_Face(ft, DEFAULT_FONT_PATH, 0, &face)) {
		ERR("Failed to load font: " << DEFAULT_FONT_PATH);
	}

	// Set the font size
	FT_Set_Pixel_Sizes(face, 0, 48);

	// Disable byte alignment restriction
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/*
	for (char c = 0; c < 128; c++) {
		// Load the bitmap for a glyph
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			ERR("Freetype: failed to load glyph");
		}
		// Generate texture

	}
	*/
}

void TextRenderer::shutdown() {

}
