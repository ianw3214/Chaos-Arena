#include "game.hpp"

#include <thread>

#include "engine/core.hpp"
#include "utils/utils.hpp"

#include "common.hpp"

#include "game/map.hpp"

// Threads
std::thread			Game::t_serverPacketSender;
std::thread			Game::t_serverPacketReciever;

// Static class variables
std::atomic_bool	Game::ready;
bool				Game::running;
int					Game::camera_x;
int					Game::camera_y;
float				Game::screen_scale;
std::mutex			Game::player_mutex;
Player				Game::player;
std::unordered_map<int, Unit>			Game::units;
std::unordered_map<int, UnitMovement>	Game::movements;

Socket::Socket		Game::socket;
int					Game::client_id;
bool				Game::connected;
std::mutex			Game::packet_queue_mutex;
std::queue<Socket::BasicPacket>			Game::packet_queue;
int					Game::packet_delta;
int					Game::packet_last_tick;

// TEMPORARY CODE
Map map;

// Temporary code, TODO: (Ian) Get rid of this
#define TILE_PATH "res/assets/tiles/tile.png"
#define TILE_SRC_W 64
#define TILE_SRC_H 64

void Game::init() {
	ready = false;
	running = true;
	camera_x = 0;
	camera_y = 0;
	player.init();
	
	// Create a socket for both sending and listening
	socket = Socket::create();
	Socket::bind(socket);
	// Socket::setNonBlock(socket);

	// Start the thread to send packets
	t_serverPacketSender = std::thread(Game::packetSender);
	// Start the thread to recieve server packets
	t_serverPacketReciever = std::thread(Game::serverPacketListener);

	map.clearMapData();

	setScreenScale(2.f);
}

void Game::shutdown() {

	// Wait for the threads to finish
	t_serverPacketSender.join();
	t_serverPacketReciever.join();

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
		std::lock_guard<std::mutex> lock(player_mutex);
		// Handle keyboard state
		const Uint8 * state = SDL_GetKeyboardState(NULL);
		player.handleEvent(e);
		if (state[SDL_SCANCODE_SPACE]) {
			map.generate();
		}
	}
	if (ready) {
		player.update(delta, UNIT_PER_TILE, map);
		// Update unit movements on each update as well
		for (auto it = movements.begin(); it != movements.end(); ++it) {
			// Skip self
			if ((*it).first == client_id) continue;
			int timestamp = SDL_GetTicks();
			// If the unit exists, update its movement if necessary
			if (units.find((*it).first) != units.end()) {
				// if ((*it).second.goal_x == (*it).second.start_x && (*it).second.goal_y == (*it).second.start_y) continue;
				Unit& unit = units.at((*it).first);
				int x = lerp((*it).second.start_x, (*it).second.goal_x, static_cast<float>(timestamp - (*it).second.timestamp) / SERVER_INTERVAL);
				int y = lerp((*it).second.start_y, (*it).second.goal_y, static_cast<float>(timestamp - (*it).second.timestamp) / SERVER_INTERVAL);
				if ((*it).second.start_x == (*it).second.goal_x && (*it).second.start_y == (*it).second.goal_y) {
					x = unit.getX();
					y = unit.getY();
				}
				unit.move(x, y);
			}
			// Otherwise, create the unit
			else {
				units[(*it).first] = Unit((*it).second.start_x, (*it).second.start_y, screen_scale);
				// Initialize any other unit properties
				units[(*it).first].setSprite();
			}
		}

		// Just center the camera around the player unit for now
		camera_x = player.getScreenX() - Engine::getScreenWidth() / 2;
		camera_y = player.getScreenY() - Engine::getScreenHeight() / 2;
		// Send any packets that the player has
		while (player.hasPacket()) {
			addPacket(player.getNextPacket());
		}
		// Send the player position if the time is up
		packet_delta = SDL_GetTicks() - packet_last_tick;
		if (packet_delta >= static_cast<int>(1000.f / PACKETS_PER_SEC)) {
			packet_last_tick = SDL_GetTicks();
			std::lock_guard<std::mutex> lock(player_mutex);
			Socket::Packet3i packet;
			packet.first = client_id;
			packet.second = player.getX();
			packet.third = player.getY();
			addPacket(Socket::createBasicPacket(packet));
		}
	}

}

void Game::render() {
	if (ready) {
		// Render the map
		map.render(camera_x, camera_y, screen_scale);
		// Render units in the map
		for (const auto& unit_pair : units) {
			if (unit_pair.first == client_id) continue;
			const Unit& unit = unit_pair.second;
			unit.render(camera_x, camera_y);
		}
		// Render the player
		std::lock_guard<std::mutex> lock(player_mutex);
		player.render(camera_x, camera_y);

		// Render the player UI
		player.renderUI();
		// Debug renders
		// map.render_debug();
	} else {
		// TODO: (Ian) Implement full loading screen
		Renderer::drawTexture({ Engine::getScreenWidth() / 2 - 64, Engine::getScreenHeight() / 2 - 64 }, 128, 128, *TextureManager::getTexture("res/assets/loading.png"));
	}

}

bool Game::isRunning() {
	return running;
}

void Game::setScreenScale(float scale) {
	screen_scale = scale;
	for (auto& unit : units) {
		unit.second.setScreenScale(scale);
	}
	player.setScreenScale(scale);
}

// Simple thread to send any packets in the queue
void Game::packetSender() {

	Socket::Address dest(68, 183, 119, 157, Socket::DEFAULT_PORT);

	{	// Send a connection packet
		Socket::Packet1i con_request = { PACKET_MSG_CONNECT };
		Socket::BasicPacket packet = Socket::createBasicPacket(con_request);
		Socket::send(socket, dest, packet);
	}

	// Wait until the game is connected AND ready
	while (!connected) std::this_thread::sleep_for(std::chrono::milliseconds(100));
	while (!ready) std::this_thread::sleep_for(std::chrono::microseconds(100));

	while (running) {
		// Send packets when they arrive in the queue
		if (hasPacket()) {
			Socket::send(socket, dest, getPacket());
		}
	}

	{	// Send a client disconnect and close the socket
		Socket::Packet2i dc_request = { PACKET_MSG_DISCONNECT, client_id };
		Socket::BasicPacket packet = Socket::createBasicPacket(dc_request);
		Socket::send(socket, dest, packet);
	}

	Socket::close(socket);

}

void Game::serverPacketListener() {

	Clock connect_timeout;
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
		// Send a new connect packet if not recieved
		// TODO: (Ian) Get rid of this magic number, maybe use a define
		if (connect_timeout.getTicks() > 3000) {
			Socket::Address dest(68, 183, 119, 157, Socket::DEFAULT_PORT);
			Socket::Packet1i con_request = { PACKET_MSG_CONNECT };
			Socket::BasicPacket packet = Socket::createBasicPacket(con_request);
			Socket::send(socket, dest, packet);
		}
	}

	while (running) {
		Socket::Packet<Socket::BasicPacket> packet = Socket::recieve<Socket::BasicPacket>(socket);
		if (packet.has_data) {
			if (Socket::getPacketType(packet.data) == PACKET_1I) {
				Socket::Packet1i con_packet = Socket::convertPacket1i(packet.data);
				if (con_packet.val == PACKET_DUNGEON_READY) {
					// Set a flag here to indicate ready
					// TODO: (Ian) Move this to a function somewhere
					map.generateTilemap();
					std::lock_guard<std::mutex> lock(player_mutex);
					Vec2i spawn_coords = map.tileToPixelCoords(map.getSpawnPoint());
					player.init_properties(client_id, spawn_coords.x, spawn_coords.y);
					// TODO: (Ian) Set this based on a ID sent from the server or something
					map.setTileSheet(TILE_PATH, TILE_SRC_W, TILE_SRC_H);
					ready = true;
					packet_delta = packet_last_tick = SDL_GetTicks();
				}
			}
			if (Socket::getPacketType(packet.data) == PACKET_2I) {
				Socket::Packet2i con_packet = Socket::convertPacket2i(packet.data);
				if (con_packet.first == PACKET_MSG_DISCONNECT) {
					for (auto it = units.begin(); it != units.end(); ++it) {
						if ((*it).first == con_packet.second) {
							LOG("Client: " << (*it).first << " disconnected.");
							int id = (*it).first;
							units.erase(id);
							movements.erase(id);
							break;
						}
					}
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
				if (con_packet.vals[0] == PACKET_PLAYER_ATTACK) {
					int id = con_packet.vals[1];
					bool face_right = con_packet.vals[3] == FACE_RIGHT ? true : false;
					// TODO: (Ian) Set face_right of unit as well
					units[id].attack_primary();
				}
			}
		}
	}
}

bool Game::hasPacket() {
	packet_queue_mutex.lock();
	bool temp = !packet_queue.empty();
	packet_queue_mutex.unlock();
	return temp;
}

void Game::addPacket(const Socket::BasicPacket packet) {
	packet_queue_mutex.lock();
	packet_queue.emplace(packet);
	packet_queue_mutex.unlock();
	return;
}

Socket::BasicPacket Game::getPacket() {
	packet_queue_mutex.lock();
	Socket::BasicPacket temp = packet_queue.front();
	packet_queue.pop();
	packet_queue_mutex.unlock();
	return temp;
}

