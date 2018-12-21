#pragma once

#include "gameUtils.hpp"
#include "engine/utils.hpp"

// TODO: (Ian) remove these things
#define PLAYER_WIDTH	50
#define PLAYER_HEIGHT	70

// Base class representing a unit in the game
class Unit {

public:

	Unit(int x=0, int y=0);
	~Unit();

	void render() const;

	void move(int x, int y);
	void move(Direction dir, int distance);

	// Getter methods
	int getX() const { return x; }
	int getY() const { return y; }
	Vec2i getPos() const { return Vec2i{ x, y }; }

private:

	int x;
	int y;

};