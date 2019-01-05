#pragma once

#include "socket.hpp"
#include "common.hpp"

#include <mutex>
#include <queue>
#include <set>
#include <thread>
#include <atomic>

struct PacketWrapper {
    Socket::BasicPacket packet;
    Socket::Address address;
    PacketWrapper(Socket::BasicPacket packet, Socket::Address address) 
        : packet(packet), address(address) {}
};

/*
 *  The interface class that handles incoming/outgoing packets
 */
class Interface {

public:

    Interface(int port=Socket::DEFAULT_PORT);
    ~Interface();

    void setNonBlock();

    void sendPacket(Socket::Packet1i packet, Socket::Address address);
    void sendPacket(Socket::Packet2i packet, Socket::Address address);
    void sendPacket(Socket::Packet3i packet, Socket::Address address);
    void sendPacket(Socket::Packetvi packet, Socket::Address address);
    void sendPacket(Socket::BasicPacket packet, Socket::Address address);
    void sendPacketGuarantee(Socket::Packet1i packet, Socket::Address address);
    void sendPacketGuarantee(Socket::Packet2i packet, Socket::Address address);
    void sendPacketGuarantee(Socket::Packet3i packet, Socket::Address address);
    void sendPacketGuarantee(Socket::Packetvi packet, Socket::Address address);
    void sendPacketGuarantee(Socket::BasicPacket packet, Socket::Address address);

    Socket::Packet<Socket::BasicPacket> recieve();

private:

    std::mutex      m_send_lock;
    std::queue<PacketWrapper> packets;

    int             m_port;
    Socket::Socket  m_socket;

    int             m_packet_id;

    std::mutex      m_response_lock;
    std::set<int>   m_expected_response;

    // The function that continuously sends packets
    std::atomic<bool>   m_running;
    std::thread         t_packet_sender;
    void f_packet_sender();

};