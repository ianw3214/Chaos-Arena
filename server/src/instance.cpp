#include "instance.hpp"

#include <thread>
#include <chrono>

Instance::Instance(const Socket::Socket& socket) : socket(socket) {


    // Start the thread to send clients player info
    std::thread t_clientSender(&Instance::clientSender, this);
    t_clientSender.detach();
}

Instance::~Instance() {
    
}

void Instance::packetRecieved(Socket::Packet<Socket::BasicPacket> packet) {
    if (Socket::getPacketType(packet.data) == PACKET_1I) {
        Socket::Packet1i packet1i = Socket::convertPacket1i(packet.data);
        if (packet1i.val == PACKET_MSG_CONNECT) {
            // Generate a new player ID and give it to the player
            int clientId = rand() % 10000;
            // Change the packet address to the default port
            clients.push_back({clientId, packet.address, 0, 0});
            Socket::Packet1i response = { clientId };
            Socket::BasicPacket con_response = Socket::createBasicPacket(response);
            Socket::send(socket, packet.address, con_response);
            LOG("Accepted client connection; client ID: " << clientId);
        }
    }
    if (Socket::getPacketType(packet.data) == PACKET_2I) {
        Socket::Packet2i packet2i = Socket::convertPacket2i(packet.data);
        if (packet2i.first == PACKET_MSG_DISCONNECT) {
            // Remove the player ID from the connect clients list
            for (auto it = clients.begin(); it != clients.end(); ++it) {
                if ((*it).m_id == packet2i.second) {
                    // TODO: Also send a message to each client about a disconnect
                    LOG("Client: " << packet2i.second << " disconnected.");
                    clients.erase(it);
                    break;
                }
            }
        }
    }
    if (Socket::getPacketType(packet.data) == PACKET_3I) {
        // Update the corresponding player position
        Socket::Packet3i packet3i = Socket::convertPacket3i(packet.data);
        // TODO: (Ian) Use a better lookup data structure for better performance
        for (ClientUnit& client : clients) {
            if (client.m_id == packet3i.first) {
                client.m_x = packet3i.second;
                client.m_y = packet3i.third;
                // LOG("Updated client: " << client.m_id << " TO (" << client.m_x << ", " << client.m_y << ")");
                break;
            }
        }
    }
}

void Instance::clientSender() {

    while (true) {
        // LOG("SENDING CLIENT PACKETS");
        // create the client packet
        Socket::Packetvi packet;
        for (const ClientUnit& client : clients) {
            packet.vals.push_back(client.m_id);
            packet.vals.push_back(client.m_x);
            packet.vals.push_back(client.m_y);
        }
        Socket::BasicPacket con_packet = Socket::createBasicPacket(packet);
        for (const ClientUnit& client : clients) {
            Socket::send(socket, client.m_address, con_packet);
        }
        // TODO: (Ian) Use a delta time calculation to determine send frequency
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

}