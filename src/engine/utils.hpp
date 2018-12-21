#pragma once

#include <vector>
#include <sstream>

struct Vec2i {
	int x;
	int y;
};
struct Vec2f {
	float x;
	float y;
};
struct Vec3i {
	int x;
	int y;
	int z;
};
struct Vec3f {
	float x;
	float y;
	float z;
};

struct Rect {
	Vec2i pos;
	int w;
	int h;
};

typedef Vec3f Colour;

namespace Util {

	// String tokenizing utility function
	typedef std::vector<std::string> StrTokens;
	inline StrTokens split(const std::string& s, char delimiter = ' ', bool removeEmpty = false) {
		StrTokens tokens;
		std::string token;
		std::istringstream tokenStream(s);
		while (std::getline(tokenStream, token, delimiter)) {
			if (removeEmpty && token.size() == 0) continue;
			tokens.push_back(token);
		}
		return tokens;
	}

}