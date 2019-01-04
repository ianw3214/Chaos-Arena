#pragma once

#include "gameUtils.hpp"
#include "engine/core.hpp"

#include <mutex>

// TODO: (Ian) remove these things
#define PLAYER_WIDTH	60
#define PLAYER_HEIGHT	100

#define DEFAULT_SPRITE	"res/assets/units/unit.png"
#define DEFAULT_FRAME_W	60
#define DEFAULT_FRAME_H 100

#define UNIT_ANIM_IDLE_RIGHT	0
#define UNIT_ANIM_IDLE_LEFT		1
#define UNIT_ANIM_RUN_RIGHT		2
#define UNIT_ANIM_RUN_LEFT		3
#define UNIT_ANIM_PUNCH_RIGHT	4
#define UNIT_ANIM_PUNCH_LEFT	5

#define ATTACK_TIMER_DEFAULT	500

class Map;

// Base class representing a unit in the game
class Unit {

public:

	Unit(int x=0, int y=0);
	~Unit();

	// Sprite/animation functions
	void setSprite(const std::string& name=DEFAULT_SPRITE, int frame_w=DEFAULT_FRAME_W, int frame_h=DEFAULT_FRAME_H);
	void addAnimation(unsigned int start, unsigned int end);
	void playAnimation(unsigned int anim, bool restart = true, unsigned int loops = 1);
	void queueAnimation(unsigned int anim, unsigned int loops = 1);

	void render(int cam_x = 0, int cam_y = 0) const;

	// Common unit functionality
	void move(int x, int y, bool update_anim = true);
	void move(Direction dir, int distance);
	void move(Direction dir, int distance, const Map& map);

	// Attack functions just handle attack visuals
	// For now, all units have the same attacks
	void attack_primary();

	// Getter methods
	int getX() const { return x; }
	int getY() const { return y; }
	Vec2i getPos() const { return Vec2i{ x, y }; }
	bool isFaceRight() const { return face_right; }

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
	bool attacking;
	Clock attack_timer;

	// The sprite of the unit
	mutable AnimatedSprite sprite;

};