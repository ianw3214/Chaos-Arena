#include "map.hpp"

#include "delaunator.hpp"
#include <cmath>

// Helper function to generate a random point in a circle
const double pi = 3.1415926535897;
Vec2i getRandomPointInCircle(int radius) {
	double t = 2.0 * pi * (static_cast<double>(rand()) / RAND_MAX);
	double r = (static_cast<double>(rand())) / RAND_MAX;
	return Vec2i{ static_cast<int>(r * std::cos(t) * radius), static_cast<int>(r * std::sin(t) * radius) };
}

std::vector<Socket::BasicPacket> Map::generatePackets() {
	std::vector<Socket::BasicPacket> packets;
	// Turn all the main rooms into basic packets
	for (const Room& room : main_rooms) {
		Socket::Packetvi packet;
		packet.vals.push_back(PACKET_DATA_ROOM);
		packet.vals.push_back(room.pos.x);
		packet.vals.push_back(room.pos.y);
		packet.vals.push_back(room.w);
		packet.vals.push_back(room.h);
		packets.push_back(Socket::createBasicPacket(packet));
	}
	// Turn all the hallways into basic packets
	for (const Edge& edge : hallways) {
		Socket::Packetvi packet;
		packet.vals.push_back(PACKET_DATA_HALLWAY);
		packet.vals.push_back(edge.v1.x);
		packet.vals.push_back(edge.v1.y);
		packet.vals.push_back(edge.v2.x);
		packet.vals.push_back(edge.v2.y);
		packets.push_back(Socket::createBasicPacket(packet));
	}
	// Send a packet for spawn room data as well
	{
		const Room& spawn_room = main_rooms[spawn_room_index];
		Socket::Packet3i packet;
		packet.first = PACKET_DATA_SPAWNPOINT;
		packet.second = spawn_room.pos.x + rand() % spawn_room.w;
		packet.third = spawn_room.pos.y + rand() % spawn_room.h;
		packets.push_back(Socket::createBasicPacket(packet));
	}
	return packets;
}

void Map::generate() {
	bool rooms_generated = false;
	do {
		// Clear previous state
		rooms.clear();
		main_rooms.clear();
		edges.clear();
		hallways.clear();

		// Try to generate a valid room layout
		generateRooms();
		if (separateRooms()) {
			rooms_generated = true;
		}
	} while (!rooms_generated);

	chooseMainRooms();
	generateEdges();
	generateHallways();

	generateTilemap();

	generateSpawnRoom();

}


void Map::generateRooms() {
	for (int i = 0; i < NUM_GENERATED_ROOMS; ++i) {
		Vec2i pos = getRandomPointInCircle(GEN_RADIUS);
		int width = rand() % MAX_WIDTH + 1;
		int height = rand() % MAX_HEIGHT + 1;
		while (width < MIN_WIDTH) width = rand() % MAX_WIDTH + 1;
		while (height < MIN_HEIGHT) height = rand() % MAX_HEIGHT + 1;
		rooms.push_back(Room{ i, pos, width, height });
	}
}

void Map::chooseMainRooms() {
	// Bubble sort because I'm lazy
	for (unsigned int i = 0; i < rooms.size(); ++i) {
		for (unsigned int j = 0; j < rooms.size(); ++j) {
			if (i >= j) continue;
			int stamp_i = rooms[i].w * rooms[i].h;
			int stamp_j = rooms[j].w * rooms[j].h;
			if (stamp_i < stamp_j) {
				std::swap(rooms[i], rooms[j]);
			}
		}
	}
	for (int i = 0; i < NUM_MAIN_ROOMS; ++i) {
		main_rooms.push_back(rooms[i]);
	}
}

bool Map::separateRooms() {
	int tries = 0;
	// Move the rooms so that they don't collide anymore
	bool has_collision = true;
	while (has_collision) {
		bool collision_found = false;
		for (unsigned int i = 0; i < rooms.size(); ++i) {
			Vec2f vec = Vec2f();
			for (unsigned int j = 0; j < rooms.size(); ++j) {
				if (i == j) continue;
				Room& r1 = rooms[i];
				Room& r2 = rooms[j];
				// If the two rooms have the same center, move one randomly
				if (r1.pos.x + r1.w / 2 == r2.pos.x + r2.w / 2 && r1.pos.y + r1.h /2 == r2.pos.y + r2.h / 2) {
					if (r1.pos.x >= 0)	r1.pos.x++;
					else				r1.pos.x--;
				}
				// If there is a collision, Add the vector to a central vector
				if (r1.pos.x < r2.pos.x + r2.w && r1.pos.x + r1.w > r2.pos.x &&
					r1.pos.y < r2.pos.y + r2.h && r1.pos.y + r1.h > r2.pos.y)
				{
					vec.x += (static_cast<float>(r2.w) / 2.f + r2.pos.x) - (static_cast<float>(r1.w) / 2.f + r1.pos.x);
					vec.y += (static_cast<float>(r2.h) / 2.f + r2.pos.y) - (static_cast<float>(r1.h) / 2.f + r1.pos.y);
					collision_found = true;
				}
			}
			if (collision_found) {
				// Normalize the vector and then calculate movement if necessary
				float magnitude = static_cast<float>(std::sqrt(vec.x * vec.x + vec.y * vec.y));
				if (magnitude == 0.f) continue;
				vec.x *= -1.f / magnitude;
				vec.y *= -1.f / magnitude;
				// arbitrary threshold
				if (std::abs(vec.x) > 0.3f && vec.x >= 0) rooms[i].pos.x++;
				if (std::abs(vec.x) > 0.3f && vec.x <= 0) rooms[i].pos.x--;
				if (std::abs(vec.y) > 0.3f && vec.y >= 0) rooms[i].pos.y++;
				if (std::abs(vec.y) > 0.3f && vec.y <= 0) rooms[i].pos.y--;
			}
		}
		has_collision = collision_found;
		if (++tries > 100) return false;
	}
	return true;
}

void Map::generateEdges() {
	// Construct the points representing the rooms
	std::vector<double> coords;
	for (const Room& room : main_rooms) {
		coords.push_back(static_cast<double>(room.pos.x + room.w / 2));
		coords.push_back(static_cast<double>(room.pos.y + room.h / 2));
	}

	// triangulation happens here
	delaunator::Delaunator d(coords);

	for (std::size_t i = 0; i < d.triangles.size(); i += 3) {
		// 3 edges per triangle
		Vec2i v1 = Vec2i{ static_cast<int>(d.coords[2 * d.triangles[i]]), static_cast<int>(d.coords[2 * d.triangles[i] + 1]) };
		Vec2i v2 = Vec2i{ static_cast<int>(d.coords[2 * d.triangles[i + 1]]), static_cast<int>(d.coords[2 * d.triangles[i + 1] + 1]) };
		Vec2i v3 = Vec2i{ static_cast<int>(d.coords[2 * d.triangles[i + 2]]), static_cast<int>(d.coords[2 * d.triangles[i + 2] + 1]) };
		int room_id1 = getRoomIdAt(v1);
		int room_id2 = getRoomIdAt(v2);
		int room_id3 = getRoomIdAt(v3);
		Edge edge1 = { room_id1, room_id2, v1, v2 };
		Edge edge2 = { room_id2, room_id3, v2, v3 };
		Edge edge3 = { room_id3, room_id1, v3, v1 };
		bool edge1_found = false;
		bool edge2_found = false;
		bool edge3_found = false;
		for (const Edge& edge : edges) {
			if (edge1 == edge) edge1_found = true;
			if (edge2 == edge) edge2_found = true;
			if (edge3 == edge) edge3_found = true;
		}
		if (!edge1_found) edges.push_back(edge1);
		if (!edge2_found) edges.push_back(edge2);
		if (!edge3_found) edges.push_back(edge3);
	}

}


void Map::generateHallways() {
	for (const Edge& edge : edges) {
		// Skip error cases
		if (edge.room_id1 < 0 || edge.room_id2 < 0) continue;

		// First calculate the midway point of the two rooms
		const Room& room1 = getRoomById(edge.room_id1);
		const Room& room2 = getRoomById(edge.room_id2);
		Vec2i room1_mid = { room1.pos.x + room1.w / 2, room1.pos.y + room1.h / 2};
		Vec2i room2_mid = { room2.pos.x + room2.w / 2, room2.pos.y + room2.h / 2 };
		int mid_x = room1_mid.x + (room2_mid.x - room1_mid.x) / 2;
		int mid_y = room1_mid.y + (room2_mid.y - room1_mid.y) / 2;

		// Check if the two rooms are close enough horizontally
		if (mid_y > room1.pos.y && mid_y < room1.pos.y + room1.h && mid_y > room2.pos.y && mid_y < room2.pos.y + room2.h) {
			int x_1 = room1.pos.x > room2.pos.x ? room1.pos.x : room1.pos.x + room1.w;
			int x_2 = room2.pos.x > room1.pos.x ? room2.pos.x : room2.pos.x + room2.w;
			hallways.push_back({ -1, -1, Vec2i{ x_1, mid_y }, Vec2i{ x_2, mid_y } });
			continue;
		}

		// Check if the two rooms are close enough vertically
		if (mid_x > room1.pos.x && mid_x < room1.pos.x + room1.w && mid_x > room2.pos.x && mid_x < room2.pos.x + room2.w) {
			int y_1 = room1.pos.y > room2.pos.y ? room1.pos.y : room1.pos.y + room1.h;
			int y_2 = room2.pos.y > room1.pos.y ? room2.pos.y : room2.pos.y + room2.h;
			hallways.push_back({ -1, -1, Vec2i{ mid_x, y_1 }, Vec2i{ mid_x, y_2 } });
			continue;
		}

		// If not any of the above, create an L shape
		// If the hallway collides with the rooms, just let it be
		{
			Edge edge1;
			edge1.v1.x = room1_mid.x > room2_mid.x ? room1.pos.x : room1.pos.x + room1.w;
			edge1.v1.y = room1_mid.y;
			edge1.v2.x = room2_mid.x;
			edge1.v2.y = room1_mid.y;
			Edge edge2;
			edge2.v1.x = room2_mid.x;
			edge2.v1.y = room1_mid.y;
			edge2.v2.x = room2_mid.x;
			edge2.v2.y = room2_mid.y > room1_mid.y ? room2.pos.y : room2.pos.y + room2.h;
			hallways.push_back(edge1);
			hallways.push_back(edge2);
		}
	}
}

void Map::generateSpawnRoom() {
	spawn_room_index = rand() % main_rooms.size();
}

#define TILE_AT(x, y, width, min_x, min_y) ((y - min_y) * width + (x - min_x))
void Map::generateTilemap() {
	// Clear previous state
	tilemap.clear();
	// First find the max x, min x, max y and min y of the map
	int max_x = 0, min_x = 0;
	int max_y = 0, min_y = 0;
	for (const Room& room : main_rooms) {
		if (room.pos.x + room.w > max_x) max_x = room.pos.x + room.w;
		if (room.pos.x < min_x) min_x = room.pos.x;
		if (room.pos.y + room.h > max_y) max_y = room.pos.y + room.h;
		if (room.pos.y < min_y) min_y = room.pos.y;
	}
	// Create the tilemap and initalize everything to 0
	tilemap = std::vector<int>((max_x - min_x + 1) * (max_y - min_y + 1), 0);
	// Start filling in the tilemap with rooms
	map_width	= max_x - min_x + 1;
	map_height	= max_y - min_y + 1;
	left_x		= min_x;
	top_y		= min_y;
	for (const Room& room : main_rooms) {
		for (int y = 0; y < room.h; ++y) {
			for (int x = 0; x < room.w; ++x) {
				tilemap[TILE_AT(room.pos.x + x, room.pos.y + y, map_width, min_x, min_y)] = 1;
			}
		}
	}
	// Then fill in the tilemap with hallways
	for (const Edge& hall : hallways) {
		// TODO: (Ian) Maybe fill in the missing corners of connected hallways
		// Horizontal hallway
		if (hall.v1.y == hall.v2.y) {
			int left = hall.v1.x < hall.v2.x ? hall.v1.x : hall.v2.x;
			for (int i = 0; i < std::abs(hall.v1.x - hall.v2.x); ++i) {
				tilemap[TILE_AT(left + i, hall.v1.y, map_width, min_x, min_y)] = 1;
				// Assume adding the height is always ok because of how room sizes work
				tilemap[TILE_AT(left + i, hall.v1.y - 1, map_width, min_x, min_y)] = 1;
				tilemap[TILE_AT(left + i, hall.v1.y + 1, map_width, min_x, min_y)] = 1;
			}
		}
		// Vertical hallway
		if (hall.v1.x == hall.v2.x) {
			int top = hall.v1.y < hall.v2.y ? hall.v1.y : hall.v2.y;
			for (int i = 0; i < std::abs(hall.v1.y - hall.v2.y); ++i) {
				tilemap[TILE_AT(hall.v1.x, top + i, map_width, min_x, min_y)] = 1;
				// Assume adding the width is always ok because of how room sizes work
				tilemap[TILE_AT(hall.v1.x - 1, top + i, map_width, min_x, min_y)] = 1;
				tilemap[TILE_AT(hall.v1.x + 1, top + i, map_width, min_x, min_y)] = 1;
			}
		}
	}
}

int Map::getRoomIdAt(Vec2i pos) {
	for (const Room& room : rooms) {
		if (pos.x > room.pos.x && pos.x < room.pos.x + room.w && pos.y > room.pos.y && pos.y < room.pos.y + room.h) {
			return room.id;
		}
	}
	return -1;
}

Room Map::getRoomById(int id) {
	for (const Room& room : rooms) {
		if (room.id == id) {
			return room;
		}
	}
	return { -1, {0, 0}, 0, 0 };
}