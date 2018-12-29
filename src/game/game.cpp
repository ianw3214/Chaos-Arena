#include "game.hpp"

#include <thread>

#include "engine/core.hpp"
#include "utils/utils.hpp"

#include "common.hpp"

#include "game/map.hpp"

// Static class variables
bool			Game::ready;
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
	ready = false;
	running = true;
	camera_x = 0;
	camera_y = 0;
	player = Unit();

	// Create a socket for both sending and listening
	socket = Socket::create();
	Socket::bind(socket);

	// TODO: (Ian) Join these threads in the end instead of detaching them
	// Start the thread to send player position
	std::thread sendPlayerPosThread(Game::playerPosSender);
	sendPlayerPosThread.detach();
	// Start the thread to recieve server packets
	std::thread serverPacketReciever(Game::serverPacketListener);
	serverPacketReciever.detach();

	map.clearMapData();
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

	if (ready) {
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
			player.move(Direction::UP, static_cast<int>(PLAYER_SPEED * UNIT_PER_TILE / delta), map);
		}
		if (state[SDL_SCANCODE_S] || state[SDL_SCANCODE_DOWN]) {
			player.move(Direction::DOWN, static_cast<int>(PLAYER_SPEED * UNIT_PER_TILE / delta), map);
		}
		if (state[SDL_SCANCODE_D] || state[SDL_SCANCODE_RIGHT]) {
			player.move(Direction::RIGHT, static_cast<int>(PLAYER_SPEED * UNIT_PER_TILE / delta), map);
		}
		if (state[SDL_SCANCODE_A] || state[SDL_SCANCODE_LEFT]) {
			player.move(Direction::LEFT, static_cast<int>(PLAYER_SPEED * UNIT_PER_TILE / delta), map);
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
			// If the unit exists, update its movement if necessary
			if (units.find((*it).first) != units.end()) {
				if ((*it).second.goal_x == (*it).second.start_x && (*it).second.goal_y == (*it).second.start_y) continue;
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

		// Just center the camera around the player unit for now
		camera_x = player.getX() - Engine::getScreenWidth() / 2;
		camera_y = player.getY() - Engine::getScreenHeight() / 2;
	}

}

void Game::render() {
	// Render the map
	map.render(camera_x, camera_y);
	// Render units in the map
	for (const auto& unit_pair : units) {
		if (unit_pair.first == client_id) continue;
		const Unit& unit = unit_pair.second;
		unit.render(camera_x, camera_y);
	}
	// Render the player
	std::lock_guard<std::mutex> lock(player_mutex);
	player.render(camera_x, camera_y);

	// Debug renders
	// map.render_debug();
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
			if (Socket::getPacketType(packet.data) == PACKET_1I) {
				Socket::Packet1i con_packet = Socket::convertPacket1i(packet.data);
				if (con_packet.val == PACKET_DUNGEON_READY) {
					// Set a flag here to indicate ready
					map.generateTilemap();
					Vec2i spawn_coords = map.tileToPixelCoords(map.getSpawnPoint());
					player.move(spawn_coords.x, spawn_coords.y);
					ready = true;
				}
			}
			if (Socket::getPacketType(packet.data) == PACKET_3I) {
				Socket::Packet3i con_packet = Socket::convertPacket3i(packet.data);
				if (con_packet.first == PACKET_DATA_SPAWNPOINT) {
					map.setSpawnPoint(con_packet.second, con_packet.third);
				}
			}
			if (Socket::getPacketType(packet.data) == PACKET_VI) {
				Socket::Packetvi con_packet = Socket::convertPacketvi(packet.data);
				if (con_packet.vals.size() <= 0) continue;
				if (con_packet.vals[0] == PACKET_PLAYER_POS) {
					for (unsigned int i = 1; i < con_packet.vals.size(); i += 3) {
						// Update the unit movement if it already exists
						int id = con_packet.vals[i];
						int x = con_packet.vals[i + 1];
						int y = con_packet.vals[i + 2];
						int timestamp = SDL_GetTicks();
						if (movements.find(id) != movements.end()) {
							// If the unit is already in the right position, ignore the packet
							if (x == movements[id].goal_x && y == movements[id].goal_y) {
								movements[id].start_x = units[id].getX();
								movements[id].start_y = units[id].getY();
								movements[id].goal_x = units[id].getX();
								movements[id].goal_y = units[id].getY();
							} else {
								movements[id].start_x = units[id].getX();
								movements[id].start_y = units[id].getY();
								movements[id].goal_x = x;
								movements[id].goal_y = y;
							}
							movements[id].timestamp = timestamp;
						} else {
							UnitMovement temp = UnitMovement{ x, y, x, y, timestamp };
							movements[id] = temp;
						}
					}
				}
				if (con_packet.vals[0] == PACKET_DATA_ROOM) {
					// Add a room to the map
					if (con_packet.vals.size() < 5) continue;	// ERROR
					int x = con_packet.vals[1];
					int y = con_packet.vals[2];
					int w = con_packet.vals[3];
					int h = con_packet.vals[4];
					map.addMainRoom({ 0, {x, y}, w, h });
				}
				if (con_packet.vals[0] == PACKET_DATA_HALLWAY) {
					// Add a hallway edge to the map
					if (con_packet.vals.size() < 5) continue;	// ERROR
					int x1 = con_packet.vals[1];
					int y1 = con_packet.vals[2];
					int x2 = con_packet.vals[3];
					int y2 = con_packet.vals[4];
					map.addHallwayEdge({ 0, 0, { x1, y1 }, { x2, y2 } });
				}
			}
		}
	}
}
