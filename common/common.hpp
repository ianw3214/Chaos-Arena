#pragma once

#include <cstring>
#include <vector>

#define PACKET_1I   0
#define PACKET_2I   1
#define PACKET_3I   2
#define PACKET_VI   3
typedef char PacketType;

// Some defines to make things easier
#define PACKET_ID_RESPONSE      0

#define PACKET_MSG_CONNECT      1
#define PACKET_MSG_DISCONNECT   2

#define PACKET_DATA_ROOM        3
#define PACKET_DATA_HALLWAY     4
#define PACKET_DATA_SPAWNPOINT  5
#define PACKET_EXPECTED_PACKETS 6
#define PACKET_PACKETS_RECIEVED 7
#define PACKET_DUNGEON_READY    8

#define PACKET_PLAYER_POS       9
#define PACKET_PLAYER_ATTACK    10
#define PACKET_PLAYER_DASH      11

// The defines for the attacks are in a separate file
#include "attacks.hpp"

#define PACKET_UNIT_DAMAGED     12
#define PACKET_UNIT_DEAD        13
#define PACKET_UNIT_RESPAWN     14

#define PACKET_PLAYER_KILL      15

// Helper function for writing data + updating pointer for packets
template<class T>
void writeData(const T& data, char*& ptr) {
    memcpy(ptr, &(data), sizeof(T));
    ptr += sizeof(T);
}

template<class T>
void getData(const char*& ptr, T& data) {
    memcpy(&data, ptr, sizeof(T));
    ptr += sizeof(T);
}

namespace Socket {

	// The basic packet, used for arbitrary information
    struct BasicPacket {
        PacketType type;
        int packet_id;
        char message[61];
    };

    struct Packet1i {
        int val;
    };

    struct Packet2i {
        int first;
        int second;
    };

    struct Packet3i {
        int first;
        int second;
        int third;
    };

    struct Packetvi {
        std::vector<int> vals;
    };

    // General function to get the type of a packet
    inline PacketType getPacketType(const BasicPacket& packet) {
        return packet.type;
    }

    // Function to determine whether the packet is guaranteed
    inline bool isPacketGuaranteed(const BasicPacket& packet) {
        return packet.packet_id != 0;
    }

    // Function to get the id of a packet
    inline int getPacketId(const BasicPacket& packet) {
        return packet.packet_id;
    }

    // Functions to turn host packet into basic network packets
    inline BasicPacket createBasicPacket(const Packet1i& packet, int packet_id = 0) {
        BasicPacket result;
        result.type = PACKET_1I;
        result.packet_id = packet_id;
        char * ptr = result.message;
        writeData(packet.val, ptr);
        return result;
    }
    inline BasicPacket createBasicPacket(const Packet2i& packet, int packet_id = 0) {
        BasicPacket result;
        result.type = PACKET_2I;
        result.packet_id = packet_id;
        char * ptr = result.message;
        writeData(packet.first, ptr);
        writeData(packet.second, ptr);
        return result;
    }
    inline BasicPacket createBasicPacket(const Packet3i& packet, int packet_id = 0) {
        BasicPacket result;
        result.type = PACKET_3I;
        result.packet_id = packet_id;
        char * ptr = result.message;
        writeData(packet.first, ptr);
        writeData(packet.second, ptr);
        writeData(packet.third, ptr);
        return result;
    }
    inline BasicPacket createBasicPacket(const Packetvi& packet, int packet_id = 0) {
        BasicPacket result;
        result.type = PACKET_VI;
        result.packet_id = packet_id;
        char * ptr = result.message;
        writeData(static_cast<int>(packet.vals.size()), ptr);
        for (int i : packet.vals) {
            writeData(i, ptr);
        }
        return result;
    }

    // Functions to turn network packets into host packet
    // NOTE: requires the type be read and correct corrsesponding function to be called
    inline Packet1i convertPacket1i(const BasicPacket& packet) {
        Packet1i result;
        const char * ptr = packet.message;
        getData(ptr, result.val);
        return result;
    }
    inline Packet2i convertPacket2i(const BasicPacket& packet) {
        Packet2i result;
        const char * ptr = packet.message;
        getData(ptr, result.first);
        getData(ptr, result.second);
        return result;
    }
    inline Packet3i convertPacket3i(const BasicPacket& packet) {
        Packet3i result;
        const char * ptr = packet.message;
        getData(ptr, result.first);
        getData(ptr, result.second);
        getData(ptr, result.third);
        return result;
    }
    inline Packetvi convertPacketvi(const BasicPacket& packet) {
        Packetvi result;
        const char * ptr = packet.message;
        unsigned int size;
        getData(ptr, size);
        result.vals.reserve(size);
        for (unsigned int i = 0; i < size; ++i) {
            int data;
            getData(ptr, data);
            result.vals.push_back(data);
        }
        return result;
    }

};