#include "player.hpp"

#include "map.hpp"

#include "utils/utils.hpp"

Player::Player(int x, int y) {
	move_up = false;
	move_down = false;
	move_right = false;
	move_left = false;
}

Player::~Player() {
	delete unit;
}

void Player::init() {
	if (unit) delete unit;
	unit = new Unit();
	unit->setSprite(DEFAULT_SPRITE, DEFAULT_FRAME_W, DEFAULT_FRAME_H);
}

void Player::init_positions(int spawn_x, int spawn_y) {
	unit->move(spawn_x, spawn_y);
}

void Player::render(int cam_x, int cam_y) {
	unit->render(cam_x, cam_y);
}

int Player::getX() const {
	return unit->getX();
}

int Player::getY() const {
	return unit->getY();
}

void Player::handleEvent(SDL_Event & e) {
	if (e.type == SDL_KEYDOWN) {
		switch (e.key.keysym.sym) {
		case SDLK_UP:
		case SDLK_w:
		{
			move_up = true;
		} break;
		case SDLK_DOWN:
		case SDLK_s:
		{
			move_down = true;
		} break;
		case SDLK_RIGHT:
		case SDLK_d:
		{
			move_right = true;
		} break;
		case SDLK_LEFT:
		case SDLK_a:
		{
			move_left = true;
		} break;
		default: {
			// Do nothing...
		} break;
		}
	}
	if (e.type == SDL_KEYUP) {
		switch (e.key.keysym.sym) {
		case SDLK_UP:
		case SDLK_w:
		{
			move_up = false;
		} break;
		case SDLK_DOWN:
		case SDLK_s:
		{
			move_down = false;
		} break;
		case SDLK_RIGHT:
		case SDLK_d:
		{
			move_right = false;
		} break;
		case SDLK_LEFT:
		case SDLK_a:
		{
			move_left = false;
		} break;
		default: {
			// Do nothing...
		} break;
		}
	}
}

void Player::update(int delta, int units_per_tile, const Map & map) {
	if (move_up)	unit->move(Direction::UP, static_cast<int>(PLAYER_SPEED * units_per_tile / delta), map);
	if (move_down)  unit->move(Direction::DOWN, static_cast<int>(PLAYER_SPEED * units_per_tile / delta), map);
	if (move_right) unit->move(Direction::RIGHT, static_cast<int>(PLAYER_SPEED * units_per_tile / delta), map);
	if (move_left)  unit->move(Direction::LEFT, static_cast<int>(PLAYER_SPEED * units_per_tile / delta), map);
}
