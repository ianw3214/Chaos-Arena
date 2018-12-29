#pragma once

#include <string>
#include <unordered_map>

class Texture;

class TextureManager {

public:
	static void init();
	static void shutdown();

	static Texture * getTexture(const std::string& name);
	static void addTexture(const std::string& path, const std::string& name = "");
	static void deleteTexture(const std::string& name);

private:

	static std::unordered_map<std::string, Texture*> s_textures;

};