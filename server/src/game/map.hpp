#pragma once

#include <vector>
#include "common.hpp"

// Struct to represent a 2D vector
struct Vec2i {
    int x;
    int y;
};

struct Vec2f {
    float x;
    float y;
};

// Struct to represent a room
struct Room {
	int id;
	Vec2i pos;
	int w;
	int h;
};

// Struct to represent an edge
struct Edge {
	int room_id1;
	int room_id2;
	Vec2i v1;
	Vec2i v2;
};


inline bool operator==(const Edge& edge1, const Edge& edge2) {
	if ((edge1.room_id1 == edge2.room_id2) && (edge1.room_id2 == edge2.room_id1)) return true;
	if ((edge1.room_id1 == edge2.room_id1) && (edge1.room_id2 == edge2.room_id2)) return true;
	return false;
}

// Temporary defines, move to a config file or take from input or something
// TODO: (Ian) Figure out where to put these defines
#define GEN_RADIUS			15
#define MAX_WIDTH			10
#define MAX_HEIGHT			10
#define MIN_WIDTH			4
#define MIN_HEIGHT			4
#define NUM_GENERATED_ROOMS	30
#define NUM_MAIN_ROOMS		5

/*
 *  A specialized map class for the server without client needed functions
 */
class Map {
public:

	// Generating packets to send to the client
	std::vector<Socket::BasicPacket> generatePackets();
	int numPackets();

	// Other helper methods for the instance to interface
	int getRandomSpawnX() const;
	int getRandomSpawnY() const;

    // Generation algorithm for the dungeon layout
    void generate();

private:

	// The actual map data
	int				  map_width;
	int				  map_height;
	std::vector<int>  tilemap;
	int				  spawn_room_index;

    // Map generation helper variables
    std::vector<Room> rooms;
	std::vector<Room> main_rooms;
	std::vector<Edge> edges;
	std::vector<Edge> hallways;
	int				  left_x;
	int				  top_y;

    // Main helper functions for dungeon generation
	void generateRooms();
	void chooseMainRooms();
	bool separateRooms();
	void generateEdges();
	void generateHallways();
	// Generate other properties of the dungeon
	void generateSpawnRoom();
	// Generate the tilemap from the generated rooms
	void generateTilemap();
	// Other misc helper functions
	int  getRoomIdAt(Vec2i pos);
	Room getRoomById(int id);

};