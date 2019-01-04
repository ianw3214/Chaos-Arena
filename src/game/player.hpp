#pragma once

#include <SDL2/SDL.h>

#include <queue>

#include "unit.hpp"
#include "common.hpp"

class Map;

// TODO: (Ian) Figure out a better way of storing this
#define PLAYER_SPEED		8.f

#define PLAYER_SPRITE	"res/assets/units/player.png"
#define PLAYER_FRAME_W	60
#define PLAYER_FRAME_H	100

#define HEALTH_SPRITE			"res/assets/UI/heart_small.png"
#define DEFAULT_PLAYER_HEALTH	5
#define HEART_WIDTH				50
#define HEART_HEIGHT			50

class Player {

public:

	Player(int x = 0, int y = 0);
	~Player();

	void init();
	void init_properties(int id, int spawn_x, int spawn_y);

	// Player networking code
	bool hasPacket() const;
	Socket::BasicPacket getNextPacket();

	// Player getter methods
	int getX() const;
	int getY() const;

	// Basic player methods
	void render(int cam_x = 0, int cam_y = 0);
	void handleEvent(SDL_Event& e);
	void update(int delta, int units_per_tile, const Map& map);
	void renderUI();

	// Player attack methods
	void attack_primary();

private:

	int player_id;

	// The player class acts as a controller for a unit
	Unit * unit;

	// Player properties
	int health;

	// Input flags
	bool move_up;
	bool move_down;
	bool move_left;
	bool move_right;

	// Queue of packets to send to the server
	std::queue<Socket::BasicPacket> packet_queue;
	void addPacket(Socket::BasicPacket packet);

};