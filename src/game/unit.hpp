#pragma once

#include "gameUtils.hpp"
#include "engine/core.hpp"

#include <mutex>

// TODO: (Ian) remove these things
#define PLAYER_WIDTH	50
#define PLAYER_HEIGHT	70

#define DEFAULT_SPRITE	"res/assets/units/unit.png"
#define DEFAULT_FRAME_W	64
#define DEFAULT_FRAME_H 128

#define UNIT_ANIM_IDLE_RIGHT	0
#define UNIT_ANIM_IDLE_LEFT		1
#define UNIT_ANIM_RUN_RIGHT		2
#define UNIT_ANIM_RUN_LEFT		3

class Map;

// Base class representing a unit in the game
class Unit {

public:

	Unit(int x=0, int y=0);
	~Unit();

	void setSprite(const std::string& name=DEFAULT_SPRITE, int frame_w=DEFAULT_FRAME_W, int frame_h=DEFAULT_FRAME_H);
	void addAnimation(unsigned int start, unsigned int end);
	void playAnimation(unsigned int anim, bool restart = true, unsigned int loops = 1);
	void queueAnimation(unsigned int anim, unsigned int loops = 1);

	void render(int cam_x = 0, int cam_y = 0) const;

	void move(int x, int y, bool update_anim = true);
	void move(Direction dir, int distance);
	void move(Direction dir, int distance, const Map& map);

	// Getter methods
	int getX() const { return x; }
	int getY() const { return y; }
	Vec2i getPos() const { return Vec2i{ x, y }; }

	// Sprite methods
	void spriteMoveUp();
	void spriteMoveDown();
	void spriteMoveRight();
	void spriteMoveLeft();
	void spriteStopMove();

protected:

	// TODO: (Ian) Separate between raw x/y and screen x/y
	int x;
	int y;

	bool face_right;

	// The sprite of the unit
	mutable AnimatedSprite sprite;

};