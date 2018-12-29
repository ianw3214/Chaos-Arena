#pragma once

#include <vector>

#include "engine/core.hpp"

/*
 *	Temporary code - move to server in the future
 */

// Temporary defines, move to a config file or take from input or something
// TODO: (Ian) Figure out where to put these defines
#define GEN_RADIUS			15
#define MAX_WIDTH			10
#define MAX_HEIGHT			10
#define MIN_WIDTH			4
#define MIN_HEIGHT			4
#define NUM_GENERATED_ROOMS	40
#define NUM_MAIN_ROOMS		10

struct Room {
	int id;
	Vec2i pos;
	int w;
	int h;
};

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

// TODO: (Ian) Move this to a class property or maybe a function parameter
#define TILE_SIZE 64

class Map {

public:

	void render(int cam_x = 0, int cam_y = 0) const;

	// Getter methods for map properties
	int   getMapWidth() const;
	int   getMapHeight() const;
	Vec2i getSpawnPoint() const;

	// Setter methods to use for incoming packets
	void clearMapData();
	void addMainRoom(Room room);
	void addHallwayEdge(Edge edge);

	// Utility methods to interact with map
	bool pointInMap(Vec2i point, int tile_size = TILE_SIZE) const;

	// Map generation algorithm, should be used by server and NOT by client
	void generate();
	// Generate the tilemap from the generated rooms
	void generateTilemap();

	// Map debugging code
	void render_debug() const;

	// TEMPORARY CODE, REMOVE LATER <======================
	// TODO: (Ian) REMOVE THIS
	void generateSpawnPoint();
	
private:

	// The actual map data
	int				  map_width;
	int				  map_height;
	std::vector<int>  tilemap;
	// Other map properties
	Vec2i spawnPoint;

	/* -----------------------------------------------------
	 * ----------------MAP GENERATION CODE------------------
	 * ----------------------------------------------------- */

	// Helper data to generate the map
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
	// Other misc helper functions
	int  getRoomIdAt(Vec2i pos);
	Room getRoomById(int id);
	bool lineColliding(Vec2i v1, Vec2i v2, Vec2i v3, Vec2i v4);

};