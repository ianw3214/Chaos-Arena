#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <map>
#include "engine/utils.hpp"

// TODO: (Ian) Configure default font folder path and only use names
#define DEFAULT_FONT_PATH	"res/fonts/OpenSans-Regular.ttf"

struct Character {
	unsigned int texture_id;
	Vec2i size;
	Vec2i bearing;
	unsigned int advance;
};

class TextRenderer {

public:
	
	static void init();
	static void shutdown();

	std::map<char, Character> characters;

};