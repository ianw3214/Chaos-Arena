#include "player.hpp"

#include "utils/utils.hpp"
#include "socket.hpp"

#include "map.hpp"
#include "number/numberSprite.hpp"

Player::Player(int x, int y) : health(DEFAULT_PLAYER_HEALTH), stamina(DEFAULT_PLAYER_STAMINA), dead(false), kills(0), deaths(0) {
	screen_scale = 1.f;
	// Initialize key flags
	move_up = false;
	move_down = false;
	move_right = false;
	move_left = false;
	attack_pressed = false;
}

Player::~Player() {
	delete unit;
}

// Just an initialization functions for sprites and shit
void Player::init() {
	// Initialize unit and it's sprites
	if (unit) delete unit;
	unit = new Unit(0, 0, screen_scale);
	unit->setSprite(PLAYER_SPRITE, PLAYER_FRAME_W, PLAYER_FRAME_H);
	unit->playAnimation(UNIT_ANIM_IDLE_RIGHT);
}

// Actual initialization of the player with properties
void Player::init_properties(int id, int spawn_x, int spawn_y) {
	dead = false;
	player_id = id;
	unit->move(spawn_x, spawn_y);
}

bool Player::hasPacket() const {
	return !packet_queue.empty();
}

Socket::BasicPacket Player::getNextPacket() {
	Socket::BasicPacket temp = packet_queue.front();
	packet_queue.pop();
	return temp;
}

void Player::addKill() {
	kills++;
}

int Player::getX() const {
	return unit->getX();
}

int Player::getY() const {
	return unit->getY();
}

int Player::getScreenX() const {
	return unit->getScreenX();
}

int Player::getScreenY() const {
	return unit->getScreenY();
}

void Player::render(int cam_x, int cam_y) {
	unit->render(cam_x, cam_y);
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
		case SDLK_z: {
			attack_pressed = true;
		} break;
		case SDLK_x: {
			dash();
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
		case SDLK_z: {
			attack_pressed = false;
		} break;
		default: {
			// Do nothing...
		} break;
		}
	}
}

void Player::update(int delta, int units_per_tile, const Map & map) {
	// Check whether to send a respawn request depending on death time
	if (dead) {
		if (!respawn_sent && respawn_timer.getTicks() > RESPAWN_TIMER) {
			Socket::Packet2i respawn_request;
			respawn_request.first = PACKET_UNIT_RESPAWN;
			respawn_request.second = player_id;
			addPacket(Socket::createBasicPacket(respawn_request));
			respawn_sent = true;
		}
	} else {
		// Regenerate stamina as long as the player isn't dead
		if (stamina_timer.getTicks() > STAMINA_TIMER) {
			stamina_timer.reset();
			if (stamina < DEFAULT_PLAYER_STAMINA) stamina++;
		}
		// Let the player attack if pressed
		if (attack_pressed) {
			attack_primary();
		}
	}
	// Also update the player movement
	if (!unit->isAttacking() && !unit->isDamaged() && !unit->isDashing() && !dead) {
		if (move_up)	unit->move(Direction::UP, static_cast<int>(PLAYER_SPEED * units_per_tile / delta), map);
		if (move_down)  unit->move(Direction::DOWN, static_cast<int>(PLAYER_SPEED * units_per_tile / delta), map);
		if (move_right) unit->move(Direction::RIGHT, static_cast<int>(PLAYER_SPEED * units_per_tile / delta), map);
		if (move_left)  unit->move(Direction::LEFT, static_cast<int>(PLAYER_SPEED * units_per_tile / delta), map);
		// Set the player to an idle state if there is no movement
		if (!move_up && !move_down && !move_right && !move_left) unit->spriteStopMove();
	}
	// Update dashing movement
	if (unit->isDashing()) {
		if (unit->dashDirection() == Direction::UP) unit->move(Direction::UP, static_cast<int>(DASH_SPEED * units_per_tile / delta), map);
		if (unit->dashDirection() == Direction::DOWN) unit->move(Direction::DOWN, static_cast<int>(DASH_SPEED * units_per_tile / delta), map);
		if (unit->dashDirection() == Direction::RIGHT) unit->move(Direction::RIGHT, static_cast<int>(DASH_SPEED * units_per_tile / delta), map);
		if (unit->dashDirection() == Direction::LEFT) unit->move(Direction::LEFT, static_cast<int>(DASH_SPEED * units_per_tile / delta), map);
	}
}

// Render UI related to the player
void Player::renderUI() {
	{	// Render the player health
		int y = Engine::getScreenHeight() - HEART_MARGIN_Y - HEART_HEIGHT;
		for (int i = 0; i < health; ++i) {
			int x = i * HEART_WIDTH + HEART_MARGIN_X;
			Renderer::drawTexture({ x, y }, HEART_WIDTH, HEART_HEIGHT, HEALTH_SPRITE);
		}
	}
	{	// Render the player stamina
		int x = STAMINA_MARGIN_X;
		int y = Engine::getScreenHeight() - STAMINA_MARGIN_Y - STAMINA_HEIGHT;
		for (int i = 0; i < stamina; ++i) {
			// Draw an outline for the stamina first
			Renderer::drawRect({ x - OUTLINE_WIDTH, y - OUTLINE_WIDTH }, STAMINA_WIDTH + OUTLINE_WIDTH * 2, STAMINA_HEIGHT + OUTLINE_WIDTH * 2, { .2f, .2f, .2f });
			Renderer::drawRect({ x, y }, STAMINA_WIDTH, STAMINA_HEIGHT, { .8f, .8f, 0.f });
			x += STAMINA_WIDTH + STAMINA_MARGIN_X;
		}
	}
	// Render player kills
	Renderer::drawTexture(Vec2i{ 700, 10 }, 250, 60, KILLS_SRC);
	Renderer::drawTexture(Vec2i{ 700, 70 }, 300, 60, DEATHS_SRC);
	Number::renderNumber(kills, 1200, 10);
	Number::renderNumber(deaths, 1200, 70);
	// Render the instructions
	Renderer::drawTexture(Vec2i{ 0, 0 }, 400, 100, INSTRUCTIONS_SRC);
}

void Player::setDead() {
	health = 0;
	dead = true;
	deaths++;
	unit->setDead();
	respawn_timer.reset();
	respawn_sent = false;
}

void Player::respawn(int x, int y) {
	unit->respawn(x, y);
	health = DEFAULT_PLAYER_HEALTH;
	stamina = DEFAULT_PLAYER_STAMINA;
	dead = false;
}

void Player::damaged() {
	// Play the animation
	unit->spriteDamaged();
	health--;
	// TODO: (Ian) Set invuln timers and what not
}

void Player::attack_primary() {
	if (!unit->isAttacking() && !unit->isDamaged() && !unit->isDashing()) {
		// Check the cooldowns
		if (attack_cooldown.getTicks() < ATTACK_COOLDOWN) return;
		attack_cooldown.reset();
		// Play the animations
		unit->attack_primary();
		// Send a packet to the server
		Socket::Packetvi packet;
		packet.vals.push_back(PACKET_PLAYER_ATTACK);
		packet.vals.push_back(player_id);
		packet.vals.push_back(ATTACK_BASIC_PUNCH);
		packet.vals.push_back(unit->isFaceRight() ? FACE_RIGHT : FACE_LEFT);
		packet.vals.push_back(unit->getX());
		packet.vals.push_back(unit->getY());
		addPacket(Socket::createBasicPacket(packet));
	}
}

void Player::dash() {
	if (!unit->isAttacking() && !unit->isDamaged() && !unit->isDashing()) {
		// Only dash if there is enough stamina left
		if (stamina <= 0) return;
		stamina--;
		stamina_timer.reset();
		// Play the animation
		if (move_right)					unit->dash(Direction::RIGHT);
		else if (move_left)				unit->dash(Direction::LEFT);
		else if (move_up)				unit->dash(Direction::UP);
		else if (move_down)				unit->dash(Direction::DOWN);
		else if (unit->isFaceRight())	unit->dash(Direction::RIGHT);
		else							unit->dash(Direction::LEFT);
		// Send a packet to the server
		Socket::Packet3i packet;
		packet.first = PACKET_PLAYER_DASH;
		packet.second = player_id;
		switch (unit->dashDirection()) {
		case Direction::UP: {
			packet.third = 0;
		} break;
		case Direction::DOWN: {
			packet.third = 1;
		} break;
		case Direction::RIGHT: {
			packet.third = 2;
		} break;
		case Direction::LEFT: {
			packet.third = 3;
		} break;
		default: {
			packet.third = 0;
		} break;
		}
		addPacket(Socket::createBasicPacket(packet));
	}
}

void Player::setScreenScale(float scale) {
	screen_scale = scale;
	if (unit) unit->setScreenScale(scale);
}

void Player::addPacket(Socket::BasicPacket packet) {
	packet_queue.push(packet);
}