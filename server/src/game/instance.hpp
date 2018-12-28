#pragma once

#include "socket.hpp"
#include "util.hpp"
#include "common.hpp"

#include <list>
#include <queue>
#include <mutex>

#include <iostream>
#define LOG(x) std::cout << "[LOG]: " << x << std::endl;
#define ERR(x) std::cerr << "[ERR]: " << x << std::endl;

// Constant variables
const std::string connect_message = std::string("connect");

struct QueuePacket {
    Socket::BasicPacket packet;
    Socket::Address address;
};

struct ClientUnit {
    int m_id;
    Socket::Address m_address;

    int m_x;
    int m_y;
};

// TODO: (Ian) Create a logger class for server logging to file
class Instance {

public:
    Instance(const Socket::Socket& i_socket);
    ~Instance();

    void packetRecieved(Socket::Packet<Socket::BasicPacket> packet);

private:

    // A queue of messages to send
    std::mutex packet_lock;
    std::queue<QueuePacket> packets;
    void queuePacket(Socket::BasicPacket packet, Socket::Address address);

    // NOTE: Using an unordered map may have better performance because we need lookup
    std::list<ClientUnit> clients;

    const Socket::Socket& socket;

    // Thread functions
    void packetSender();
    void clientSender();

};