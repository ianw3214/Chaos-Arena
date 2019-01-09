#include "game.hpp"

#include <thread>

#include "engine/core.hpp"
#include "utils/utils.hpp"

#include "common.hpp"

#include "game/map.hpp"
#include "game/network/interface.hpp"

// Constants
const Socket::Address dest(68, 183, 119, 157, Socket::DEFAULT_PORT);

// Threads
std::thread			Game::t_sendConnection;

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

Interface *			Game::network;
int					Game::client_id;
bool				Game::connected;
int					Game::packet_delta;
int					Game::packet_last_tick;

bool				Game::spawnPointRecieved;
int					Game::packetsRecieved;

// TEMPORARY CODE
Map map;

// Temporary code, TODO: (Ian) Get rid of this
#define TILE_PATH "res/assets/tiles/tile.png"
#define TILE_SRC_W 64
#define TILE_SRC_H 64

void Game::f_sendConnection() {
	// TOOD: (Ian) Limit the amount of tries here
	// Keep sending connection packets every 500 ms until server connects
	do {
		LOG("SENDING CONNECTION");
		Socket::Packet1i con_request = { PACKET_MSG_CONNECT };
		Socket::BasicPacket packet = Socket::createBasicPacket(con_request);
		network->sendPacket(packet, dest);

		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
	} while (!connected);
}

void Game::sendRecievedPackets() {
	Socket::Packet2i packet;
	packet.first = PACKET_PACKETS_RECIEVED;
	packet.second = packetsRecieved;
	network->sendPacketGuarantee(packet, dest);
}

void Game::init(Interface * net) {
	network = net;
	ready = false;
	running = true;
	camera_x = 0;
	camera_y = 0;
	player.init();
	// Helper variables for recieving dungeon data
	spawnPointRecieved = false;
	packetsRecieved = 0;
	
	// Clear the map at the beginning just to be safe
	map.clearMapData();

	// Temporary debugging code
	setScreenScale(2.f);
	
	// Start the thread to send connection packets
	// TODO: (Ian) Join this thread somewhere maybe
	t_sendConnection = std::thread(Game::f_sendConnection);
	t_sendConnection.detach();
}

void Game::shutdown() {

	{	// Send a client disconnect and close the socket
		Socket::Packet2i dc_request = { PACKET_MSG_DISCONNECT, client_id };
		Socket::BasicPacket packet = Socket::createBasicPacket(dc_request);
		network->sendPacket(packet, dest);
	}

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
			// TODO: Allow differentiation between guarantee and non guarantee packets
			network->sendPacket(player.getNextPacket(), dest);
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
			network->sendPacket(Socket::createBasicPacket(packet), dest);
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

void Game::packetRecieved(Socket::BasicPacket packet) {
	if (!connected) {
		if (Socket::getPacketType(packet) == PACKET_1I) {
			Socket::Packet1i con_response = Socket::convertPacket1i(packet);
			client_id = con_response.val;
			LOG("Recieved id from server: " << client_id);
			connected = true;
		}
	} else {
		if (Socket::getPacketType(packet) == PACKET_1I) {
			Socket::Packet1i con_packet = Socket::convertPacket1i(packet);
			if (con_packet.val == PACKET_DUNGEON_READY) {
				LOG("DUNGEON READY PACKET");
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
		if (Socket::getPacketType(packet) == PACKET_2I) {
			Socket::Packet2i con_packet = Socket::convertPacket2i(packet);
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
			if (con_packet.first == PACKET_UNIT_DAMAGED) {
				LOG("UNIT DAMAGED!");
			}
		}
		if (Socket::getPacketType(packet) == PACKET_3I) {
			Socket::Packet3i con_packet = Socket::convertPacket3i(packet);
			if (con_packet.first == PACKET_DATA_SPAWNPOINT) {
				// Set the spawn point if not yet set and send response to server
				if (!spawnPointRecieved) {
					map.setSpawnPoint(con_packet.second, con_packet.third);
					packetsRecieved++;
					sendRecievedPackets();
					spawnPointRecieved = true;
				}
			}
		}
		if (Socket::getPacketType(packet) == PACKET_VI) {
			Socket::Packetvi con_packet = Socket::convertPacketvi(packet);
			if (con_packet.vals.size() <= 0) return;
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
				if (con_packet.vals.size() < 5) return;	// ERROR
				int x = con_packet.vals[1];
				int y = con_packet.vals[2];
				int w = con_packet.vals[3];
				int h = con_packet.vals[4];
				if (!map.containsRoom(x, y, w, h)) {
					map.addMainRoom({ 0, {x, y}, w, h });
					packetsRecieved++;
					sendRecievedPackets();
				}
			}
			if (con_packet.vals[0] == PACKET_DATA_HALLWAY) {
				// Add a hallway edge to the map
				if (con_packet.vals.size() < 5) return;	// ERROR
				int x1 = con_packet.vals[1];
				int y1 = con_packet.vals[2];
				int x2 = con_packet.vals[3];
				int y2 = con_packet.vals[4];
				if (!map.containsHallway(x1, y1, x2, y2)) {
					map.addHallwayEdge({ 0, 0, { x1, y1 }, { x2, y2 } });
					packetsRecieved++;
					sendRecievedPackets();
				}
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