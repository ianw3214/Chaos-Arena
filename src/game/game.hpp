#pragma once

#include "unit.hpp"
#include "player.hpp"

#include "socket.hpp"

#include <unordered_map>
#include <mutex>
#include <atomic>

// TODO: (Ian) Figure out a better way of storing this
#define UNIT_PER_TILE	64

// Helper struct to sync player movement to the server
#define SERVER_INTERVAL	500.f
struct UnitMovement {
	int start_x, start_y;
	int goal_x, goal_y;
	int timestamp;
};

// A static class to represent game state
// TODO: (Ian) Lock things better for multithreading (mainly each indiidual unit)
class Game {

public:

	static void init();
	static void shutdown();
	static void update(int delta);
	static void render();

	static bool isRunning();

	// Multithreaded code
	// TODO: (Ian) Figure out how to stop these threads once the game exits
	// TODO: (Ian) Queue structure to send data in a centralized location
	static void playerPosSender();
	static void serverPacketListener();

private:

	static std::atomic_bool ready;
	static bool running;

	static int camera_x;
	static int camera_y;

	// TODO: (Ian) include the mutex in the unit class
	static std::mutex player_mutex;
	static Player player;
	// TODO: (Ian) Perhaps use a map or something for faster lookup
	// NOTE: Maybe a static array of size 100 works well w/ a bloom filter
	static std::unordered_map<int, Unit> units;
	static std::unordered_map<int, UnitMovement> movements;

	// Network code
	static Socket::Socket socket;
	// TODO: (Ian) Perhaps there is a better way of storing this
	static int client_id;
	// TODO: (Ian) Find a better solution instead of busy waiting
	static bool connected;

};