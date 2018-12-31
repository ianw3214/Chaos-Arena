#pragma once

#include <SDL2/SDL.h>

#include "unit.hpp"

class Map;

// TODO: (Ian) Figure out a better way of storing this
#define PLAYER_SPEED		8.f

#define PLAYER_SPRITE	"res/assets/units/player.png"
#define PLAYER_FRAME_W	64
#define PLAYER_FRAME_H	128

class Player {

public:

	Player(int x = 0, int y = 0);
	~Player();

	void init();
	void init_positions(int spawn_x, int spawn_y);

	void render(int cam_x = 0, int cam_y = 0);

	int getX() const;
	int getY() const;

	void handleEvent(SDL_Event& e);
	void update(int delta, int units_per_tile, const Map& map);

private:

	// The player class acts as a controller for a unit
	Unit * unit;

	// Input flags
	bool move_up;
	bool move_down;
	bool move_left;
	bool move_right;

};