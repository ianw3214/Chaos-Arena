#include "sprite.hpp"

#include "renderer/renderer.hpp"
#include "textureManager/textureManager.hpp"
#include "opengl/texture.hpp"

Sprite::Sprite(const std::string & name) {
	m_name = name;

	src_x = 0;
	src_y = 0;
	original_w = src_w = TextureManager::getTexture(name)->getWidth();
	original_h = src_h = TextureManager::getTexture(name)->getHeight();
}

Sprite::~Sprite() {
	// There's nothing to free...
}

void Sprite::render() const {
	// Construct the target rect
	if (m_name.size() > 0) {
		Renderer::drawSprite(*this);
	} else {
		// Render a magenta square if the source texture isn't set
		Renderer::drawRect({ x, y }, w, h, { 1.f, 0.f, 1.f });
	}
}

const Texture & Sprite::getTexture() const {
	return *TextureManager::getTexture(m_name);
}

void Sprite::setSource(const std::string & name) {
	m_name = name;
}

int Sprite::getOriginalWidth() const {
	return original_w;
}

int Sprite::getOriginalHeight() const {
	return original_h;
}

void Sprite::setPos(int x, int y) {
	this->x = x;
	this->y = y;
}

void Sprite::setSize(int w, int h) {
	this->w = w;
	this->h = h;
}

void Sprite::setSourcePos(int x, int y) {
	this->src_x = x;
	this->src_y = y;
}

void Sprite::setSourceSize(int w, int h) {
	this->src_w = w;
	this->src_h = h;
}
