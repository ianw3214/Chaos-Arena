#include "unit.hpp"

#include "engine/core.hpp"
#include "utils/utils.hpp"

#include "map.hpp"

Unit::Unit(int x, int y, float scale) : punch_sprite(PUNCH_SRC, PUNCH_WIDTH, PUNCH_HEIGHT, true) {
	this->x = x;
	this->y = y;
	setScreenScale(scale);
	calculateScreenPos();
	dead = false;
	face_right = true;
	attacking = false;
	damaged = false;

	// Set a default sprite
	setSprite();
	
	// Initialize punch sprites
	punch_sprite.addAnimation(0, 2);
	punch_sprite.addAnimation(3, 5);
	// Just play the sprite offscreen at first
	punch_sprite.setPos(-100, -100);
	punch_sprite.playAnimation(0);

	return;
}

Unit::~Unit() {
	return;
}

void Unit::setSprite(const std::string & name, int frame_w, int frame_h) {
	sprite.setSource(name);
	sprite.setFrameSize(frame_w, frame_h);

	// Hard code animation data for now
	// TODO: (Ian) load this in from a file
	addAnimation(0, 9);
	addAnimation(10, 19);
	addAnimation(20, 25);
	addAnimation(30, 35);
	addAnimation(40, 49);
	addAnimation(50, 59);
	addAnimation(60, 69);
	addAnimation(70, 79);
	addAnimation(80, 89);
	addAnimation(90, 99);
	addAnimation(100, 109);
}

void Unit::addAnimation(unsigned int start, unsigned int end) {
	sprite.addAnimation(start, end);
}

void Unit::playAnimation(unsigned int anim, bool restart, unsigned int loops) {
	// Don't restart the animation if not specified
	if (!restart && sprite.getCurrentAnimation() == anim) return;
	sprite.playAnimation(anim, loops);
}

void Unit::queueAnimation(unsigned int anim, unsigned int loops) {
	sprite.queueAnimation(anim, loops);
}

void Unit::render(int cam_x, int cam_y) const {
	// TODO: (Ian) Not hard code these maybe
	int x = screen_x - static_cast<int>(screen_scale * PLAYER_WIDTH) / 2 - cam_x;
	int y = screen_y - static_cast<int>(screen_scale * PLAYER_HEIGHT) - cam_y;
	sprite.setPos(x, y);
	sprite.render();
	int p_x = punch_x - cam_x + (punch_right ? 1 : -1) * static_cast<int>(screen_scale * PUNCH_OFFSET_X);
	if (!punch_right) p_x -= static_cast<int>(screen_scale * PUNCH_WIDTH);
	int p_y = punch_y - static_cast<int>(screen_scale * PLAYER_HEIGHT) - cam_y + static_cast<int>(screen_scale * PUNCH_OFFSET_Y);
	punch_sprite.setPos(p_x, p_y);
	punch_sprite.render();
	/*
	{	// debugging
		int x = screen_x - static_cast<int>(screen_scale * PLAYER_WIDTH) / 2 - cam_x;
		if (face_right) x += static_cast<int>(screen_scale * 30);
		else x -= static_cast<int>(screen_scale * 30) - static_cast<int>(screen_scale * PLAYER_WIDTH) + static_cast<int>(screen_scale * 70);
		int y = screen_y - static_cast<int>(screen_scale * PLAYER_HEIGHT) - cam_y + static_cast<int>(screen_scale * 20);
		int w = static_cast<int>(screen_scale * 70);
		int h = static_cast<int>(screen_scale * 80);
		Renderer::drawRectOutline({ x, y }, w, h);
	}
	*/
	return;
}

void Unit::move(int x, int y, bool update_anim) {
	// Update animations first
	if (this->x < x) {
		spriteMoveRight();
	} else if (this->x > x) {
		spriteMoveLeft();
	} else if (this->y != y) {
		// Sprite move up/down is the same logic
		spriteMoveUp();
	} else {
		spriteStopMove();
	}
	this->x = x;
	this->y = y;
	calculateScreenPos();
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
	// Update animations as well
	switch (dir) {
	case Direction::UP:		{ spriteMoveUp(); } break;
	case Direction::DOWN:	{ spriteMoveDown(); } break;
	case Direction::RIGHT:	{ spriteMoveRight(); } break;
	case Direction::LEFT:	{ spriteMoveLeft(); } break;
	}
	calculateScreenPos();
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
	// Update animations as well
	switch (dir) {
	case Direction::UP: { spriteMoveUp(); } break;
	case Direction::DOWN: { spriteMoveDown(); } break;
	case Direction::RIGHT: { spriteMoveRight(); } break;
	case Direction::LEFT: { spriteMoveLeft(); } break;
	}
	calculateScreenPos();
	return;
}

void Unit::setDead() {
	dead = true;
	sprite.playAnimation(UNIT_ANIM_DEAD);
}

void Unit::respawn(int x, int y) {
	this->x = x;
	this->y = y;
	dead = false;
	calculateScreenPos();
}

void Unit::attack_primary() {
	if (!dead) {
		if (attacking) {
			if (attack_timer.getTicks() > ATTACK_TIMER_DEFAULT) {
				attacking = false;
			}
		}
		if (damaged) {
			if (damaged_timer.getTicks() > DAMAGE_TIMER_DEFAULT) {
				damaged = false;
			}
		}
		if (dashing) {
			if (dash_timer.getTicks() > DASH_TIMER_DEFAULT) {
				dashing = false;
			}
		}
		if (!attacking && !damaged && !dashing) {
			// Play the attack animation
			if (face_right) {
				playAnimation(UNIT_ANIM_PUNCH_RIGHT);
				queueAnimation(UNIT_ANIM_IDLE_RIGHT);
			} else {
				playAnimation(UNIT_ANIM_PUNCH_LEFT);
				queueAnimation(UNIT_ANIM_IDLE_LEFT);
			}
			// Update position and play punch animation
			punch_x = screen_x;
			punch_y = screen_y;
			if (face_right) {
				punch_sprite.playAnimation(PUNCH_ANIM_RIGHT);
				punch_right = true;
			} else {
				punch_sprite.playAnimation(PUNCH_ANIM_LEFT);
				punch_right = false;
			}
			// Set the animation timer
			attacking = true;
			attack_timer.reset();
		}
	}
}

void Unit::dash(Direction direction) {
	if (!dead) {
		if (attacking) {
			if (attack_timer.getTicks() > ATTACK_TIMER_DEFAULT) {
				attacking = false;
			}
		}
		if (damaged) {
			if (damaged_timer.getTicks() > DAMAGE_TIMER_DEFAULT) {
				damaged = false;
			}
		}
		if (dashing) {
			if (dash_timer.getTicks() > DASH_TIMER_DEFAULT) {
				dashing = false;
			}
		}
		if (!attacking && !damaged && !dashing) {
			dash_direction = direction;
			if (dash_direction == Direction::LEFT) face_right = false;
			if (dash_direction == Direction::RIGHT) face_right = true;
			// Play the dash animation
			if (face_right) {
				playAnimation(UNIT_ANIM_DASH_RIGHT);
				queueAnimation(UNIT_ANIM_IDLE_RIGHT);
			} else {
				playAnimation(UNIT_ANIM_DASH_LEFT);
				queueAnimation(UNIT_ANIM_IDLE_LEFT);
			}
			// Set the animation timer
			dashing = true;
			dash_timer.reset();
		}
	}
}

bool Unit::isDamaged() {
	if (damaged) {
		if (damaged_timer.getTicks() > DAMAGE_TIMER_DEFAULT) {
			damaged = false;
		}
	}
	return damaged;
}

bool Unit::isAttacking() {
	if (attacking) {
		if (attack_timer.getTicks() > ATTACK_TIMER_DEFAULT) {
			attacking = false;
		}
	}
	return attacking;
}

bool Unit::isDashing() {
	if (dashing) {
		if (dash_timer.getTicks() > DASH_TIMER_DEFAULT) {
			dashing = false;
		}
	}
	return dashing;
}

Direction Unit::dashDirection() const {
	return dash_direction;
}

void Unit::spriteMoveUp() {
	if (!dead) {
		if (attacking) {
			if (attack_timer.getTicks() > ATTACK_TIMER_DEFAULT) {
				attacking = false;
			}
		}
		if (damaged) {
			if (damaged_timer.getTicks() > DAMAGE_TIMER_DEFAULT) {
				damaged = false;
			}
		}
		if (dashing) {
			if (dash_timer.getTicks() > DASH_TIMER_DEFAULT) {
				dashing = false;
			}
		}
		if (!attacking && !damaged && !dashing) {
			// Play the run animation based on the direction the unit is facing
			if (face_right) {
				playAnimation(UNIT_ANIM_RUN_RIGHT, false);
			} else {
				playAnimation(UNIT_ANIM_RUN_LEFT, false);
			}
		}
	}
}

void Unit::spriteMoveDown() {
	if (!dead) {
		if (attacking) {
			if (attack_timer.getTicks() > ATTACK_TIMER_DEFAULT) {
				attacking = false;
			}
		}
		if (damaged) {
			if (damaged_timer.getTicks() > DAMAGE_TIMER_DEFAULT) {
				damaged = false;
			}
		}
		if (dashing) {
			if (dash_timer.getTicks() > DASH_TIMER_DEFAULT) {
				dashing = false;
			}
		}
		if (!attacking && !damaged && !dashing) {
			// Play the run animation based on the direction the unit is facing
			if (face_right) {
				playAnimation(UNIT_ANIM_RUN_RIGHT, false);
			} else {
				playAnimation(UNIT_ANIM_RUN_LEFT, false);
			}
		}
	}
}

void Unit::spriteMoveRight() {
	if (!dead) {
		if (attacking) {
			if (attack_timer.getTicks() > ATTACK_TIMER_DEFAULT) {
				attacking = false;
			}
		}
		if (damaged) {
			if (damaged_timer.getTicks() > DAMAGE_TIMER_DEFAULT) {
				damaged = false;
			}
		}
		if (dashing) {
			if (dash_timer.getTicks() > DASH_TIMER_DEFAULT) {
				dashing = false;
			}
		}
		if (!attacking && !damaged && !dashing) {
			// Make the sprite face right and play the right animation
			face_right = true;
			playAnimation(UNIT_ANIM_RUN_RIGHT, false);
		}
	}
}

void Unit::spriteMoveLeft() {
	if (!dead) {
		if (attacking) {
			if (attack_timer.getTicks() > ATTACK_TIMER_DEFAULT) {
				attacking = false;
			}
		}
		if (damaged) {
			if (damaged_timer.getTicks() > DAMAGE_TIMER_DEFAULT) {
				damaged = false;
			}
		}
		if (dashing) {
			if (dash_timer.getTicks() > DASH_TIMER_DEFAULT) {
				dashing = false;
			}
		}
		if (!attacking && !damaged && !dashing) {
			// Make the sprite face left and play the right animation
			face_right = false;
			playAnimation(UNIT_ANIM_RUN_LEFT, false);
		}
	}
}

void Unit::spriteStopMove() {
	if (!dead) {
		if (attacking) {
			if (attack_timer.getTicks() > ATTACK_TIMER_DEFAULT) {
				attacking = false;
			}
		}
		if (damaged) {
			if (damaged_timer.getTicks() > DAMAGE_TIMER_DEFAULT) {
				damaged = false;
			}
		}
		if (dashing) {
			if (dash_timer.getTicks() > DASH_TIMER_DEFAULT) {
				dashing = false;
			}
		}
		if (!attacking && !damaged && !dashing) {
			// Play the idle animation based on the direction the unit is facing
			// Play the run animation based on the direction the unit is facing
			if (face_right) {
				playAnimation(UNIT_ANIM_IDLE_RIGHT, false);
			} else {
				playAnimation(UNIT_ANIM_IDLE_LEFT, false);
			}
		}
	}
}

void Unit::spriteDamaged() {
	if (!dead) {
		// Play the danaged animation
		if (face_right) {
			playAnimation(UNIT_ANIM_DAMAGE_RIGHT);
			queueAnimation(UNIT_ANIM_IDLE_RIGHT);
		} else {
			playAnimation(UNIT_ANIM_DAMAGE_LEFT);
			queueAnimation(UNIT_ANIM_IDLE_LEFT);
		}
		// Set the animation timer
		damaged = true;
		damaged_timer.reset();
	}
}

void Unit::setScreenScale(float scale) {
	screen_scale = scale;
	sprite.setSize(static_cast<int>(scale * PLAYER_WIDTH), static_cast<int>(scale * PLAYER_HEIGHT));
	punch_sprite.setSize(static_cast<int>(scale * PUNCH_WIDTH), static_cast<int>(scale * PUNCH_HEIGHT));
}

void Unit::calculateScreenPos() {
	screen_x = static_cast<int>(screen_scale * x);
	screen_y = static_cast<int>(screen_scale * y);
}
