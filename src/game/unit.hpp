#pragma once

#include "gameUtils.hpp"
#include "engine/core.hpp"

#include <mutex>

// TODO: (Ian) remove these things
#define PLAYER_WIDTH	50
#define PLAYER_HEIGHT	70

#define DEFAULT_UNIT_SPRITE		"res/assets/units/unit.png"
#define DEFAULT_UNIT_FRAME_W	64
#define DEFAULT_UNIT_FRAME_H	128

class Map;

// Base class representing a unit in the game
class Unit {

public:

	Unit(int x=0, int y=0);
	~Unit();

	void setSprite(const std::string& name=DEFAULT_UNIT_SPRITE, int frame_w=DEFAULT_UNIT_FRAME_W, int frame_h=DEFAULT_UNIT_FRAME_H);

	void render(int cam_x = 0, int cam_y = 0) const;

	void move(int x, int y);
	void move(Direction dir, int distance);
	void move(Direction dir, int distance, const Map& map);

	// Getter methods
	int getX() const { return x; }
	int getY() const { return y; }
	Vec2i getPos() const { return Vec2i{ x, y }; }

protected:

	// TODO: (Ian) Separate between raw x/y and screen x/y
	int x;
	int y;

	// The sprite of the unit
	mutable AnimatedSprite sprite;

};