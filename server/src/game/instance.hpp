#pragma once

#include "util.hpp"
#include "common.hpp"

#include <list>
#include <queue>
#include <mutex>
#include <chrono>

#include "../interface.hpp"
#include "map.hpp"

#include <iostream>
#define LOG(x) std::cout << "[LOG]: " << x << std::endl;
#define ERR(x) std::cerr << "[ERR]: " << x << std::endl;

#define PLAYER_POS_SEND_FREQUENCY   100

#define PLAYER_DEFAULT_HEALTH       5

// Constant variables
const std::string connect_message = std::string("connect");

struct ClientUnit {
    int m_id;
    Socket::Address m_address;

    bool ready;

    // In game player properties
    int m_x;
    int m_y;
    int m_health;
};

// TODO: (Ian) Create a logger class for server logging to file
class Instance {

public:
    Instance(Interface& network);
    ~Instance();

    void packetRecieved(Socket::Packet<Socket::BasicPacket> packet);

private:

    // NOTE: Using an unordered map may have better performance because we need lookup
    std::list<ClientUnit> clients;
    void addNewClient(Socket::Address address);
    ClientUnit * getClientById(int id);

    // Game state variables
    Map map;

    Interface& network;

    // Thread functions
    void clientSender();

};