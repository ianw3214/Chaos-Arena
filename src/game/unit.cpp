#include "unit.hpp"

#include "engine/core.hpp"

#include "map.hpp"

Unit::Unit(int x, int y) {
	this->x = x;
	this->y = y;
	return;
}

Unit::~Unit() {
	return;
}

void Unit::render(int cam_x, int cam_y) const {
	Renderer::drawRectOutline(Vec2i{ x - PLAYER_WIDTH / 2 - cam_x, y - PLAYER_HEIGHT - cam_y}, PLAYER_WIDTH, PLAYER_HEIGHT);
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

// TODO: (Ian) Profile this and decide if logic needs to be changed (Probably not)
void Unit::move(Direction dir, int distance, const Map & map) {
	// If the player is already off the map, then RIP
	if (!map.pointInMap({ x, y })) return;
	switch (dir) {
	case Direction::UP:		{ y -= distance; } break;
	case Direction::DOWN:	{ y += distance; } break;
	case Direction::LEFT:	{ x -= distance; } break;
	case Direction::RIGHT:	{ x += distance; } break;
	// Do nothing if se somehow get to default
	default: {} break;
	}
	// While the point is NOT in the map, move the player back
	while (!(map.pointInMap({ x, y }))) {
		switch (dir) {
		case Direction::UP:		{ y ++; } break;
		case Direction::DOWN:	{ y --; } break;
		case Direction::LEFT:	{ x ++; } break;
		case Direction::RIGHT:	{ x --; } break;
		// Do nothing if se somehow get to default
		default: {} break;
		}
	}
	return;
}
