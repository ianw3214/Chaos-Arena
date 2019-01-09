#include "player.hpp"

#include "map.hpp"

#include "utils/utils.hpp"
#include "socket.hpp"

Player::Player(int x, int y) : health(DEFAULT_PLAYER_HEALTH) {
	screen_scale = 1.f;
	// Initialize key flags
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
	unit = new Unit(0, 0, screen_scale);
	unit->setSprite(PLAYER_SPRITE, PLAYER_FRAME_W, PLAYER_FRAME_H);
	unit->playAnimation(UNIT_ANIM_IDLE_RIGHT);
}

void Player::init_properties(int id, int spawn_x, int spawn_y) {
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
			attack_primary();
		}
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
	// Set the player to an idle state if there is no movement
	if (!move_up && !move_down && !move_right && !move_left) unit->spriteStopMove();
}

// Render UI related to the player
void Player::renderUI() {
	// Render the player health
	int y = Engine::getScreenHeight() - HEART_HEIGHT;
	for (int i = 0; i < health; ++i) {
		int x = i * HEART_WIDTH;
		Renderer::drawTexture({ x, y }, HEART_WIDTH, HEART_HEIGHT, HEALTH_SPRITE);
	}
}

void Player::damaged() {
	// Play the animation
	unit->spriteDamaged();
	// TODO: (Ian) Set invuln timers and what not
	// TODO: (Ian) Update health here
}

void Player::attack_primary() {
	// Play the animation
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

void Player::setScreenScale(float scale) {
	screen_scale = scale;
	if (unit) unit->setScreenScale(scale);
}

void Player::addPacket(Socket::BasicPacket packet) {
	packet_queue.push(packet);
}