#include "renderer.hpp"

#include "engine/shaderLoader/shaderLoader.hpp"
#include "engine/sprite/sprite.hpp"

// Constants for rendering shapes
const unsigned int LINE_INDICES[2] = { 0, 1 };
const unsigned int SQUARE_INDICES[6] = { 0, 1, 3, 0, 2, 3 };
const unsigned int RECT_OUTLINE_INDICES[8] = { 0, 1, 1, 3, 3, 2, 2, 0 };

#define BASIC_VERT_PATH "res/shaders/screenCoord.vert"
#define BASIC_FRAG_PATH "res/shaders/singleColour.frag"
#define TEXTR_FRAG_PATH "res/shaders/texture.frag"

void Renderer::init() {
	// Initialize default shaders
	ShaderRef basicShader = ShaderLoader::loadShader("basic", BASIC_VERT_PATH, BASIC_FRAG_PATH);
	ShaderRef textureShader = ShaderLoader::loadShader("texture", BASIC_VERT_PATH, TEXTR_FRAG_PATH);
	// Set the screen size uniforms of the shaders to a default
	setScreenSize(RENDERER_DEFAULT_WIDTH, RENDERER_DEFAULT_HEIGHT);
}

void Renderer::shutdown() {
	// Do nothing
}

void Renderer::setScreenSize(int width, int height) {
	ShaderRef basicShader = ShaderLoader::getShader("basic");
	basicShader->setUniform1i("u_screen_width", 500);
	basicShader->setUniform1i("u_screen_height", 500);
	
	ShaderRef textureShader = ShaderLoader::getShader("texture");
	textureShader->setUniform1i("u_screen_width", 500);
	textureShader->setUniform1i("u_screen_height", 500);
}

void Renderer::clear(Colour colour, float alpha) {
	glClearColor(colour.x, colour.y, colour.z, alpha);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::drawLine(Vec2i v1, Vec2i v2, Colour colour) {
	int positions[4] = { v1.x, v1.y, v2.x, v2.y };
	VertexArray va;
	VertexBuffer vb(positions, sizeof(int) * 4);
	IndexBuffer ib(LINE_INDICES, 2);
	// Specify the layout of the buffer data
	VertexBufferLayout layout;
	layout.pushInt(2);
	va.addBuffer(vb, layout);

	// Set the uniform to draw the right colour
	ShaderRef basicShader = ShaderLoader::getShader("basic");
	basicShader->setUniform4f("u_colour", colour.x, colour.y, colour.z, 1.f);
	drawLines(va, ib, *basicShader);
}

void Renderer::drawRect(Rect rect, Colour colour) {
	drawRect(rect.pos, rect.w, rect.h, colour);
}

void Renderer::drawRect(Vec2i v, int width, int height, Colour colour) {
	int positions[8] = {
		v.x, v.y,
		v.x + width, v.y,
		v.x, v.y + height,
		v.x + width, v.y + height
	};
	VertexArray va;
	VertexBuffer vb(positions, sizeof(int) * 8);
	IndexBuffer ib(SQUARE_INDICES, 6);
	// Specify the layout of the buffer data
	VertexBufferLayout layout;
	layout.pushInt(2);
	va.addBuffer(vb, layout);

	// Issue the actual draw call
	ShaderRef basicShader = ShaderLoader::getShader("basic");
	basicShader->setUniform4f("u_colour", colour.x, colour.y, colour.z, 1.f);
	drawTriangles(va, ib, *basicShader);
}

void Renderer::drawRectOutline(Rect rect, Colour colour) {
	drawRectOutline(rect.pos, rect.w, rect.h, colour);
}

void Renderer::drawRectOutline(Vec2i v, int width, int height, Colour colour) {
	int positions[8] = {
		v.x, v.y,
		v.x + width, v.y,
		v.x, v.y + height,
		v.x + width, v.y + height
	};
	VertexArray va;
	VertexBuffer vb(positions, sizeof(int) * 8);
	IndexBuffer ib(RECT_OUTLINE_INDICES, 8);
	// Specify the layout of the buffer data
	VertexBufferLayout layout;
	layout.pushInt(2);
	va.addBuffer(vb, layout);

	// Issue the actual draw call
	ShaderRef basicShader = ShaderLoader::getShader("basic");
	basicShader->setUniform4f("u_colour", colour.x, colour.y, colour.z, 1.f);
	drawLineStrip(va, ib, *basicShader);
}

void Renderer::drawTexture(Vec2i v, int width, int height, const Texture & texture) {
	int positions[8] = {
		v.x, v.y,
		v.x + width, v.y,
		v.x, v.y + height,
		v.x + width, v.y + height
	};
	float textures[8] = {
		0.f, 1.f,
		1.f, 1.f,
		0.f, 0.f,
		1.f, 0.f
	};
	VertexArray		va;
	VertexBuffer	vb_pos(positions, sizeof(int) * 8);
	VertexBuffer	vb_tex(textures, sizeof(float) * 8);
	IndexBuffer		ib(SQUARE_INDICES, 6);
	// Specify the layout of the buffer data
	VertexBufferLayout layout_pos;
	layout_pos.pushInt(2);
	va.addBuffer(vb_pos, layout_pos, 0);
	VertexBufferLayout layout_tex;
	layout_tex.pushFloat(2);
	va.addBuffer(vb_tex, layout_tex, 1);

	// Bind the texture and draw
	texture.bind();
	ShaderRef textureShader = ShaderLoader::getShader("texture");
	drawTriangles(va, ib, *textureShader);
}

void Renderer::drawSprite(const Sprite & sprite) {
	int positions[8] = {
		sprite.x, sprite.y,
		sprite.x + sprite.w, sprite.y,
		sprite.x, sprite.y + sprite.h,
		sprite.x + sprite.w, sprite.y + sprite.h
	};
	// NOTE: (Ian) This could potentially be moved to shader code
	float textures[8] = {
		static_cast<float>(sprite.src_x) / static_cast<float>(sprite.getOriginalWidth()),
		static_cast<float>(sprite.getOriginalHeight() - sprite.src_y - sprite.src_h) / static_cast<float>(sprite.getOriginalHeight()),
		static_cast<float>(sprite.src_x + sprite.src_w) / static_cast<float>(sprite.getOriginalWidth()),
		static_cast<float>(sprite.getOriginalHeight() - sprite.src_y - sprite.src_h) / static_cast<float>(sprite.getOriginalHeight()),
		static_cast<float>(sprite.src_x) / static_cast<float>(sprite.getOriginalWidth()),
		static_cast<float>(sprite.getOriginalHeight() - sprite.src_y) / static_cast<float>(sprite.getOriginalHeight()),
		static_cast<float>(sprite.src_x + sprite.src_w) / static_cast<float>(sprite.getOriginalWidth()),
		static_cast<float>(sprite.getOriginalHeight() - sprite.src_y) / static_cast<float>(sprite.getOriginalHeight())
	};
	VertexArray		va;
	VertexBuffer	vb_pos(positions, sizeof(int) * 8);
	VertexBuffer	vb_tex(textures, sizeof(float) * 8);
	IndexBuffer		ib(SQUARE_INDICES, 6);
	// Specify the layout of the buffer data
	VertexBufferLayout layout_pos;
	layout_pos.pushInt(2);
	va.addBuffer(vb_pos, layout_pos, 0);
	VertexBufferLayout layout_tex;
	layout_tex.pushFloat(2);
	va.addBuffer(vb_tex, layout_tex, 1);

	// Bind the texture and draw
	sprite.getTexture().bind();
	ShaderRef textureShader = ShaderLoader::getShader("texture");
	drawTriangles(va, ib, *textureShader);
}

void Renderer::drawTriangles(const VertexArray & va, const IndexBuffer & ib, const Shader & shader) {
	draw(va, ib, shader, GL_TRIANGLES);
}

void Renderer::drawLines(const VertexArray & va, const IndexBuffer & ib, const Shader & shader) {
	draw(va, ib, shader, GL_LINES);
}

void Renderer::drawLineStrip(const VertexArray & va, const IndexBuffer & ib, const Shader & shader) {
	draw(va, ib, shader, GL_LINE_STRIP);
}

void Renderer::draw(const VertexArray & va, const IndexBuffer & ib, const Shader & shader, GLenum type) {
	shader.bind();
	va.bind();
	ib.bind();

	glDrawElements(type, ib.getCount(), GL_UNSIGNED_INT, nullptr);
}
