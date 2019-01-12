#include "game.hpp"

#include <thread>

#include "engine/core.hpp"
#include "utils/utils.hpp"

#include "common.hpp"

#include "game/map.hpp"
#include "game/network/interface.hpp"
#include "number/numberSprite.hpp"

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
int					Game::expectedPackets;
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

		std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	} while (!connected);
}

void Game::newUnit(int id, int x, int y) {
	units.insert({ id, Unit(x, y, screen_scale) });
}

Unit & Game::getUnit(int id) {
	if (units.find(id) == units.end()) {
		newUnit(id, 0, 0);
	}
	return units[id];
}

void Game::sendRecievedPackets() {
	Socket::Packet3i packet;
	packet.first = PACKET_PACKETS_RECIEVED;
	packet.second = packetsRecieved;
	packet.third = client_id;
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
	expectedPackets = -1;
	packetsRecieved = 0;
	
	// Clear the map at the beginning just to be safe
	map.clearMapData();

	// Temporary debugging code
	setScreenScale(1.5f);
	
	// Start the thread to send connection packets
	// TODO: (Ian) Join this thread somewhere maybe
	t_sendConnection = std::thread(Game::f_sendConnection);
	t_sendConnection.detach();

	// Initialize some textures
	TextureManager::addTexture("res/assets/loading.png");
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
		if (e.type == SDL_WINDOWEVENT) {
			if (e.window.event == SDL_WINDOWEVENT_CLOSE) {
				running = false;
			}
		}
		std::lock_guard<std::mutex> lock(player_mutex);
		// Handle keyboard state
		const Uint8 * state = SDL_GetKeyboardState(NULL);
		player.handleEvent(e);
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
				// Use this to prevent default construction
				newUnit((*it).first, (*it).second.start_x, (*it).second.start_y);
				// Initialize any other unit properties
				getUnit((*it).first).setSprite();
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
			Socket::Packetvi packet;
			packet.vals.push_back(PACKET_PLAYER_POS);
			packet.vals.push_back(client_id);
			packet.vals.push_back(player.getX());
			packet.vals.push_back(player.getY());
			network->sendPacket(Socket::createBasicPacket(packet), dest);
		}
	} else {
		// Quick hack for in case things don't send properly
		if (expectedPackets == packetsRecieved) {
			sendRecievedPackets();
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
		Renderer::drawTexture({ 0, 0 }, Engine::getScreenWidth(), Engine::getScreenHeight(), *TextureManager::getTexture("res/assets/loading.png"));
		// Draw the loading packet numbers
		if (expectedPackets > 0) {
			// Render the actual number on the left
			Number::renderNumber(packetsRecieved, Engine::getScreenWidth() / 2, Engine::getScreenHeight() - 100);
			// Render the expected number on the right
			Number::renderNumber(expectedPackets, Engine::getScreenWidth() - 300, Engine::getScreenHeight() - 100);
		}
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
			if (con_packet.val == PACKET_PLAYER_KILL) {
				// Give the player a kill
				player.addKill();
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
				// If the unit is the player, damage it
				if (con_packet.second == client_id) {
					player.damaged();
				} else {
					// Otherwise, find the unit and play it's damage animation
					for (auto it = units.begin(); it != units.end(); ++it) {
						if ((*it).first == con_packet.second) {
							(*it).second.spriteDamaged();
						}
					}
				}
			}
			if (con_packet.first == PACKET_UNIT_DEAD) {
				if (con_packet.second == client_id) {
					player.setDead();
				} else {
					for (auto it = units.begin(); it != units.end(); ++it) {
						if ((*it).first == con_packet.second) {
							(*it).second.setDead();
						}
					}
				}
			}
			if (con_packet.first == PACKET_EXPECTED_PACKETS) {
				expectedPackets = con_packet.second;
			}
		}
		if (Socket::getPacketType(packet) == PACKET_3I) {
			Socket::Packet3i con_packet = Socket::convertPacket3i(packet);
			if (con_packet.first == PACKET_DATA_SPAWNPOINT) {
				// Set the spawn point if not yet set and send response to server
				map.setSpawnPoint(con_packet.second, con_packet.third);
				packetsRecieved++;
				sendRecievedPackets();
				spawnPointRecieved = true;
			}
			if (con_packet.first == PACKET_PLAYER_DASH) {
				Direction dir = Direction::RIGHT;
				if (con_packet.third == 0) dir = Direction::UP;
				if (con_packet.third == 1) dir = Direction::DOWN;
				if (con_packet.third == 2) dir = Direction::RIGHT;
				if (con_packet.third == 3) dir = Direction::LEFT;
				for (auto it = units.begin(); it != units.end(); ++it) {
					if ((*it).first == con_packet.second) {
						(*it).second.dash(dir);
					}
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
							movements[id].start_x = movements[id].goal_x;
							movements[id].start_y = movements[id].goal_y;
							// No change in goal here
						} else {
							// Assume the unit is already created here
							movements[id].start_x = getUnit(id).getX();
							movements[id].start_y = getUnit(id).getY();
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
				map.addMainRoom({ 0, {x, y}, w, h });
				packetsRecieved++;
				sendRecievedPackets();
			}
			if (con_packet.vals[0] == PACKET_DATA_HALLWAY) {
				// Add a hallway edge to the map
				if (con_packet.vals.size() < 5) return;	// ERROR
				int x1 = con_packet.vals[1];
				int y1 = con_packet.vals[2];
				int x2 = con_packet.vals[3];
				int y2 = con_packet.vals[4];
				map.addHallwayEdge({ 0, 0, { x1, y1 }, { x2, y2 } });
				packetsRecieved++;
				sendRecievedPackets();
			}
			if (con_packet.vals[0] == PACKET_PLAYER_ATTACK) {
				int id = con_packet.vals[1];
				bool face_right = con_packet.vals[3] == FACE_RIGHT ? true : false;
				// TODO: (Ian) Set face_right of unit as well
				getUnit(id).attack_primary();
			}
			if (con_packet.vals[0] == PACKET_UNIT_RESPAWN) {
				if (con_packet.vals.size() < 4) return;	// ERROR
				int x = con_packet.vals[2];
				int y = con_packet.vals[3];
				Vec2i spawn_coords = map.tileToPixelCoords(Vec2i{ x, y });
				if (con_packet.vals[1] == client_id) {
					// Respawn the player
					player.respawn(spawn_coords.x, spawn_coords.y);
				} else {
					// Respawn the unit
					for (auto it = units.begin(); it != units.end(); ++it) {
						if ((*it).first == con_packet.vals[1]) {
							(*it).second.respawn(spawn_coords.x, spawn_coords.y);
						}
					}
				}
			}
		}
	}
}