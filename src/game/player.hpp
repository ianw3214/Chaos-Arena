#pragma once

#include <SDL2/SDL.h>

#include <queue>

#include "unit.hpp"
#include "common.hpp"

class Map;

// TODO: (Ian) Figure out a better way of storing this
#define PLAYER_SPEED	8.f
#define DASH_SPEED		16.f

#define PLAYER_SPRITE	"res/assets/units/player.png"
#define PLAYER_FRAME_W	60
#define PLAYER_FRAME_H	100

#define HEALTH_SPRITE			"res/assets/UI/heart_small.png"
#define DEFAULT_PLAYER_STAMINA	3
#define DEFAULT_PLAYER_HEALTH	5
#define HEART_WIDTH				50
#define HEART_HEIGHT			50
#define HEART_MARGIN_X			30
#define HEART_MARGIN_Y			20

#define STAMINA_HEIGHT			10
#define STAMINA_WIDTH			80
#define STAMINA_MARGIN_X		20
#define STAMINA_MARGIN_Y		10
#define OUTLINE_WIDTH			2

#define ATTACK_COOLDOWN			320
#define RESPAWN_TIMER			4000
#define STAMINA_TIMER			1500

#define KILLS_SRC				"res/assets/UI/kills.png"
#define DEATHS_SRC				"res/assets/UI/deaths.png"
#define INSTRUCTIONS_SRC		"res/assets/UI/instructions.png"

class Player {

public:

	Player(int x = 0, int y = 0);
	~Player();

	void init();
	void init_properties(int id, int spawn_x, int spawn_y);

	// Player networking code
	bool hasPacket() const;
	Socket::BasicPacket getNextPacket();
	void addKill();

	// Unit interactions
	int getX() const;
	int getY() const;
	int getScreenX() const;
	int getScreenY() const;
	void setScreenScale(float scale);

	// Basic player methods
	void render(int cam_x = 0, int cam_y = 0);
	void handleEvent(SDL_Event& e);
	void update(int delta, int units_per_tile, const Map& map);
	void renderUI();
	void setDead();
	void respawn(int x, int y);

	// Player attack methods
	void damaged();
	void attack_primary();
	void dash();

private:

	int player_id;

	float screen_scale;

	// The player class acts as a controller for a unit
	Unit * unit;

	// Player properties
	int health;
	int stamina;
	bool dead;
	int kills;
	int deaths;

	// Timers
	bool respawn_sent;
	Clock attack_cooldown;
	Clock respawn_timer;
	Clock stamina_timer;

	// Input flags
	bool move_up;
	bool move_down;
	bool move_left;
	bool move_right;
	bool attack_pressed;

	// Queue of packets to send to the server
	std::queue<Socket::BasicPacket> packet_queue;
	void addPacket(Socket::BasicPacket packet);

};