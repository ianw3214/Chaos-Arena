#include "game.hpp"

#include <thread>

#include "engine/core.hpp"
#include "utils/utils.hpp"

#include "common.hpp"

#include "game/map.hpp"

// Static class variables
bool			Game::running;
int				Game::camera_x;
int				Game::camera_y;
std::mutex		Game::player_mutex;
Unit			Game::player;
std::unordered_map<int, Unit>			Game::units;
std::unordered_map<int, UnitMovement>	Game::movements;

Socket::Socket	Game::socket;
int				Game::client_id;
bool			Game::connected;

// TEMPORARY CODE
Map map;

void Game::init() {
	running = true;
	camera_x = 0;
	camera_y = 0;
	player = Unit(250, 250);

	// Create a socket for both sending and listening
	socket = Socket::create();
	Socket::bind(socket);

	// Start the thread to send player position
	std::thread sendPlayerPosThread(Game::playerPosSender);
	sendPlayerPosThread.detach();
	// Start the thread to recieve server packets
	std::thread serverPacketReciever(Game::serverPacketListener);
	serverPacketReciever.detach();

	map.generate();
}

void Game::shutdown() {
	// Clean up resources

	// Send a client disconnect and close the socket
	Socket::Address dest(68, 183, 119, 157, Socket::DEFAULT_PORT);
	Socket::Packet2i con_request = { PACKET_MSG_DISCONNECT, client_id };
	Socket::BasicPacket packet = Socket::createBasicPacket(con_request);
	Socket::send(socket, dest, packet);
}

// TODO: Put this somewhere else
template <typename T>
inline T lerp(T v0, T v1, float t) {
	return static_cast<T>((1 - t)*v0 + t * v1);
}

void Game::update(int delta) {
	SDL_Event e;
	while (SDL_PollEvent(&e) > 0) {
		if (e.type == SDL_KEYDOWN) {
			if (e.key.keysym.sym == SDLK_ESCAPE) {
				running = false;
			}
		}
	}

	std::lock_guard<std::mutex> lock(player_mutex);
	// Handle keyboard state
	const Uint8 * state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_W] || state[SDL_SCANCODE_UP]) {
		player.move(Direction::UP, static_cast<int>(PLAYER_SPEED / delta));
	}
	if (state[SDL_SCANCODE_S] || state[SDL_SCANCODE_DOWN]) {
		player.move(Direction::DOWN, static_cast<int>(PLAYER_SPEED / delta));
	}
	if (state[SDL_SCANCODE_D] || state[SDL_SCANCODE_RIGHT]) {
		player.move(Direction::RIGHT, static_cast<int>(PLAYER_SPEED / delta));
	}
	if (state[SDL_SCANCODE_A] || state[SDL_SCANCODE_LEFT]) {
		player.move(Direction::LEFT, static_cast<int>(PLAYER_SPEED / delta));
	}
	if (state[SDL_SCANCODE_SPACE]) {
		map.generate();
	}
	if (state[SDL_SCANCODE_U]) {
		camera_y--;
	}
	if (state[SDL_SCANCODE_J]) {
		camera_y++;
	}
	if (state[SDL_SCANCODE_K]) {
		camera_x++;
	}
	if (state[SDL_SCANCODE_H]) {
		camera_x--;
	}

	// Update unit movements on each update as well
	for (auto it = movements.begin(); it != movements.end(); ++it) {
		int timestamp = SDL_GetTicks();
		// If the unit exists, update its movement
		if (units.find((*it).first) != units.end()) {
			Unit& unit = units.at((*it).first);
			int x = lerp((*it).second.start_x, (*it).second.goal_x, static_cast<float>(timestamp - (*it).second.timestamp) / SERVER_INTERVAL);
			int y = lerp((*it).second.start_y, (*it).second.goal_y, static_cast<float>(timestamp - (*it).second.timestamp) / SERVER_INTERVAL);
			unit.move(x, y);
		}
		// Otherwise, create the unit
		else {
			units[(*it).first] = Unit((*it).second.start_x, (*it).second.start_y);
		}
	}
}

void Game::render() {
	/*
	// Render units in the map
	for (const auto& unit_pair : units) {
		if (unit_pair.first == client_id) continue;
		const Unit& unit = unit_pair.second;
		unit.render();
	}
	// Render the player
	std::lock_guard<std::mutex> lock(player_mutex);
	player.render();
	*/
	map.render(camera_x, camera_y);
	map.render_debug();
}

bool Game::isRunning() {
	return running;
}

#define PACKETS_PER_SEC	15
void Game::playerPosSender() {

	// Send a request connection packet first
	// TODO: (Ian) Move the destination address to a const somewhere
	Socket::Address dest(68, 183, 119, 157, Socket::DEFAULT_PORT);
	Socket::Packet1i con_request = { PACKET_MSG_CONNECT };
	Socket::BasicPacket packet = Socket::createBasicPacket(con_request);
	Socket::send(socket, dest, packet);

	int packet_delta;
	int packet_last_tick;

	while (!connected) std::this_thread::sleep_for(std::chrono::milliseconds(100));

	packet_delta = packet_last_tick = SDL_GetTicks();
	while (true) {
		packet_delta = SDL_GetTicks() - packet_last_tick;
		if (packet_delta < static_cast<int>(1000.f / PACKETS_PER_SEC)) continue;
		packet_last_tick = SDL_GetTicks();
		std::lock_guard<std::mutex> lock(player_mutex);
		Socket::Packet3i packet;
		packet.first = client_id;
		packet.second = player.getX();
		packet.third = player.getY();
		Socket::send(socket, dest, Socket::createBasicPacket(packet));
	}

	Socket::close(socket);
}

void Game::serverPacketListener() {

	while (!connected) {
		// Wait for the message and get the player ID
		Socket::Packet<Socket::BasicPacket> response = Socket::recieve<Socket::BasicPacket>(socket);
		if (response.has_data) {
			if (Socket::getPacketType(response.data) == PACKET_1I) {
				Socket::Packet1i con_response = Socket::convertPacket1i(response.data);
				client_id = con_response.val;
				LOG("Recieved id from server: " << client_id);
				connected = true;
			}
		}
	}

	while (true) {
		Socket::Packet<Socket::BasicPacket> packet = Socket::recieve<Socket::BasicPacket>(socket);
		if (packet.has_data) {
			if (Socket::getPacketType(packet.data) == PACKET_VI) {
				// TODO: (Ian) Add a packet type to the packets to determine what to parse it as
				// NOTE: For now, parse the packets as movement packets
				/*	
				Socket::Packetvi con_packet = Socket::convertPacketvi(packet.data);
				for (unsigned int i = 0; i < con_packet.vals.size(); i += 3) {
					// Update the unit if it already exists
					if (units.find(con_packet.vals[i]) != units.end()) {
						units[con_packet.vals[i]].move(con_packet.vals[i + 1], con_packet.vals[i + 2]);
					} else {
						units[con_packet.vals[i]] = Unit(con_packet.vals[i + 1], con_packet.vals[i + 2]);
					}
				}
				*/
				Socket::Packetvi con_packet = Socket::convertPacketvi(packet.data);
				for (unsigned int i = 0; i < con_packet.vals.size(); i += 3) {
					// Update the unit movement if it already exists
					int id = con_packet.vals[i];
					int x = con_packet.vals[i + 1];
					int y = con_packet.vals[i + 2];
					int timestamp = SDL_GetTicks();
					if (movements.find(id) != movements.end()) {
						// ASSUME UNITS[ID] ALREADY EXISTS
						movements[id].start_x = units[id].getX();
						movements[id].start_y = units[id].getY();
						movements[id].goal_x = x;
						movements[id].goal_y = y;
						movements[id].timestamp = timestamp;
					} else {
						UnitMovement temp = UnitMovement{ x, y, x, y, timestamp };
						movements[id] = temp;
					}
				}
			}
		}
	}
}
