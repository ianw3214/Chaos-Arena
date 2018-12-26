#pragma once

#include "engine/opengl/glwrappers.hpp"

#include "engine/utils.hpp"

class Renderer {

public:
	static void init();
	static void shutdown();

	static void clear(Colour colour = {0.f, 0.f, 0.f}, float alpha = 1.f);
	
	static void drawLine(Vec2i v1, Vec2i v2, Colour colour = {1.f, 0.f, 0.f});
	static void drawRect(Rect rect, Colour colour = { 1.f, 0.f, 0.f });
	static void drawRect(Vec2i v, int width, int height, Colour colour = { 1.f, 0.f, 0.f });
	static void drawRectOutline(Rect rect, Colour colour = { 1.f, 0.f, 0.f });
	static void drawRectOutline(Vec2i v, int width, int height, Colour colour = { 1.f, 0.f, 0.f });
	static void drawTexture(Vec2i v1, int width, int height, const Texture& texture);

	static void drawTriangles(const VertexArray& va, const IndexBuffer& ib, const Shader& shader);
	static void drawLines(const VertexArray& va, const IndexBuffer& ib, const Shader& shader);
	static void drawLineStrip(const VertexArray& va, const IndexBuffer& ib, const Shader& shader);
	static void draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader, GLenum type);

};