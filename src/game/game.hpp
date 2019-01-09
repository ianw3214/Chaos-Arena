#pragma once

#include "unit.hpp"
#include "player.hpp"

#include "common.hpp"

#include <unordered_map>
#include <mutex>
#include <atomic>
#include <thread>

class Interface;

// TODO: (Ian) Figure out a better way of storing this
#define UNIT_PER_TILE	64

#define PACKETS_PER_SEC	15

// Helper struct to sync player movement to the server
#define SERVER_INTERVAL	100.f
struct UnitMovement {
	int start_x, start_y;
	int goal_x, goal_y;
	int timestamp;
};

// A static class to represent game state
// TODO: (Ian) Lock things better for multithreading (mainly each indiidual unit)
class Game {

public:

	static void init(Interface * interface);
	static void shutdown();
	static void update(int delta);
	static void render();

	static bool isRunning();
	static void setScreenScale(float scale);

	// Packet recieving code
	static void packetRecieved(Socket::BasicPacket packet);

	// Connection packet sending thread
	static void f_sendConnection();
	static std::thread t_sendConnection;

private:

	static std::atomic_bool ready;
	static bool running;

	static int camera_x;
	static int camera_y;
	static float screen_scale;

	// TODO: (Ian) include the mutex in the unit class
	static std::mutex player_mutex;
	static Player player;
	// TODO: (Ian) Perhaps use a map or something for faster lookup
	// NOTE: Maybe a static array of size 100 works well w/ a bloom filter
	static std::unordered_map<int, Unit> units;
	static std::unordered_map<int, UnitMovement> movements;

	// Network code
	static Interface * network;
	// TODO: (Ian) Perhaps there is a better way of storing this
	static int client_id;
	// TODO: (Ian) Find a better solution instead of busy waiting
	static bool connected;
	static int packet_delta;
	static int packet_last_tick;

};