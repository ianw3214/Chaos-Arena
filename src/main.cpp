#include <iostream>
#include <thread>
#include <mutex>

#include "engine/core.hpp"
#include "utils/utils.hpp"

#include "game/game.hpp"
#include "game/network/interface.hpp"

// TODO: (Ian) Figure out where to move this
int delta;
int last_tick;

int main(int argc, char* argv[]) {

	// TODO: (Ian) More error checking
	Engine::init();
	Engine::resize(1280, 720);
	if (!Socket::init()) {
		ERR("Couldn't initialize sockets");
		exit(1);
	}
	
	{	// The actual running of the game
		Interface network;
		network.setNonBlock();

		Game::init(&network);

		// Initialize delta and last_tick
		delta = last_tick = SDL_GetTicks();

		while (Game::isRunning()) {
			// Recieve packets on the network and pass them to the game
			Socket::Packet<Socket::BasicPacket> packet = network.recieve();
			if (packet.has_data) {
				Game::packetRecieved(packet.data);
			}
			// Then detemine if the game state needs to be updated
			delta = SDL_GetTicks() - last_tick;
			if (delta < 33) continue;
			last_tick = SDL_GetTicks();

			Renderer::clear();
			Game::update(delta);
			Game::render();
			Engine::swapScreenBuffer();
		}

		Game::shutdown();
	}
	
	Engine::shutdown();
	Socket::shutdown();

	return 0;

}