#include "textureManager.hpp"

#include "../opengl/texture.hpp"
#include "utils/utils.hpp"

std::unordered_map<std::string, Texture*> TextureManager::s_textures;

void TextureManager::init() {
	// Do nothing...
}

void TextureManager::shutdown() {
	// Do nothing...
}

Texture * TextureManager::getTexture(const std::string & name) {
	if (s_textures.find(name) == s_textures.end()) {
		// Try to load the texture here
		addTexture(name);
	}
	return s_textures[name];
}

void TextureManager::addTexture(const std::string & path, const std::string & name) {
	if (name.size() > 0) {
		s_textures[name] = new Texture(path);
	} else {
		s_textures[path] = new Texture(path);
	}
}

void TextureManager::deleteTexture(const std::string & name) {
	s_textures.erase(name);
}
