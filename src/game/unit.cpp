#include "unit.hpp"

#include "engine/core.hpp"

Unit::Unit(int x, int y) {
	this->x = x;
	this->y = y;
	return;
}

Unit::~Unit() {
	return;
}

void Unit::render() const {
	Renderer::drawRectOutline(Vec2i{ x - PLAYER_WIDTH / 2, y - PLAYER_HEIGHT / 2 }, PLAYER_WIDTH, PLAYER_HEIGHT);
	return;
}

void Unit::move(int x, int y) {
	this->x = x;
	this->y = y;
	return;
}

void Unit::move(Direction dir, int distance) {
	switch (dir) {
	case Direction::UP:		{ y -= distance; } break;
	case Direction::DOWN:	{ y += distance; } break;
	case Direction::LEFT:	{ x -= distance; } break;
	case Direction::RIGHT:	{ x += distance; } break;
		// Do nothing if se somehow get to default
	default: {} break;
	}
	return;
}