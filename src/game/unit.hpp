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
#define UNIT_ANIM_DAMAGE_RIGHT	6
#define UNIT_ANIM_DAMAGE_LEFT	7
#define UNIT_ANIM_DEAD			8
#define UNIT_ANIM_DASH_RIGHT	9
#define UNIT_ANIM_DASH_LEFT		10

#define ATTACK_TIMER_DEFAULT	220
#define DAMAGE_TIMER_DEFAULT	400
#define DASH_TIMER_DEFAULT		300

#define PUNCH_SRC				"res/assets/units/punch.png"
#define PUNCH_WIDTH				50
#define PUNCH_HEIGHT			70
#define PUNCH_OFFSET_X			20
#define PUNCH_OFFSET_Y			30
#define PUNCH_ANIM_RIGHT		0
#define PUNCH_ANIM_LEFT			1

class Map;

// Base class representing a unit in the game
class Unit {

public:

	Unit(int x=0, int y=0, float screen_scale = 1.f);
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
	void setDead();
	void respawn(int x, int y);

	// Attack functions just handle attack visuals
	// For now, all units have the same attacks
	void attack_primary();
	void dash(Direction direction);

	// Getter methods
	int getX() const { return x; }
	int getY() const { return y; }
	int getScreenX() const { return screen_x; }
	int getScreenY() const { return screen_y; }
	Vec2i getPos() const { return Vec2i{ x, y }; }
	bool isFaceRight() const { return face_right; }
	bool isDamaged();
	bool isAttacking();
	bool isDashing();
	Direction dashDirection() const;

	// Sprite methods
	void spriteMoveUp();
	void spriteMoveDown();
	void spriteMoveRight();
	void spriteMoveLeft();
	void spriteStopMove();
	void spriteDamaged();
	void setScreenScale(float scale);
	void calculateScreenPos();

protected:

	// TODO: (Ian) Separate between raw x/y and screen x/y
	int x;
	int y;

	int screen_x;
	int screen_y;
	float screen_scale;

	// Unit properties
	bool dead;
	bool face_right;
	// Timers
	bool attacking;
	Clock attack_timer;
	bool damaged;
	Clock damaged_timer;
	bool dashing;
	Direction dash_direction;
	Clock dash_timer;

	// The sprite of the unit
	mutable AnimatedSprite sprite;

	// Other sprites that may be needed
	mutable AnimatedSprite punch_sprite;
	int punch_x, punch_y;
	bool punch_right;

};