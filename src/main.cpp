#include <iostream>
#include <thread>
#include <mutex>

#include "engine/core.hpp"
#include "utils/utils.hpp"

#include "game/game.hpp"

// TODO: (Ian) Figure out where to move this
int delta;
int last_tick;

int main(int argc, char* argv[]) {

	// TODO: (Ian) More error checking
	Engine::init();
	if (!Socket::init()) {
		ERR("Couldn't initialize sockets");
		exit(1);
	}
	
	Game::init();
	
	// Initialize delta and last_tick
	delta = last_tick = SDL_GetTicks();

	while(Game::isRunning()) {
		delta = SDL_GetTicks() - last_tick;
		if (delta < 33) continue;
		last_tick = SDL_GetTicks();
		
		Renderer::clear();

		Game::update(delta);
		Game::render();

		Engine::swapScreenBuffer();
	}

	Game::shutdown();
	Engine::shutdown();
	Socket::shutdown();

	return 0;

}