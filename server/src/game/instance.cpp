#include "instance.hpp"

#include <thread>
#include <chrono>

Instance::Instance(Interface& network) : network(network) {

    // Initialize instance properties
    map.generate();
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
            // Add the client trying to connect
            addNewClient(packet.address);
            // Send dungeon generation data to the client
            auto packets = map.generatePackets();
            for (Socket::BasicPacket& map_packet : packets) {
                network.sendPacketGuarantee(map_packet, packet.address);
            }
        }
    }
    if (Socket::getPacketType(packet.data) == PACKET_2I) {
        Socket::Packet2i packet2i = Socket::convertPacket2i(packet.data);
        if (packet2i.first == PACKET_MSG_DISCONNECT) {
            // Remove the player ID from the connect clients list
            for (auto it = clients.begin(); it != clients.end(); ++it) {
                if ((*it).m_id == packet2i.second) {
                    LOG("Client: " << packet2i.second << " disconnected.");
                    clients.erase(it);
                    break;
                }
            }
            // Send the same disconnect packet back to the clients
            for (const ClientUnit& client : clients) {
                network.sendPacket(packet2i, client.m_address);
            }
        }
        if (packet2i.first == PACKET_PACKETS_RECIEVED) {
            if (packet2i.second == map.numPackets()) {
                // Send a ready packet if the player has recieved all the packets
                Socket::Packet1i ready_packet = { PACKET_DUNGEON_READY };
                network.sendPacketGuarantee(ready_packet, packet.address);
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
                break;
            }
        }
    }
    if (Socket::getPacketType(packet.data) == PACKET_VI) {
        Socket::Packetvi packetvi = Socket::convertPacketvi(packet.data);
        // TODO: (Ian) Better erorr handling
        if (packetvi.vals.size() == 0) return;
        if (packetvi.vals[0] == PACKET_PLAYER_ATTACK) {
            LOG("PLAYER ATTACK");
            if (packetvi.vals[2] == ATTACK_BASIC_PUNCH) {
                if (packetvi.vals.size() < 6) return;
                std::vector<int> collisions;
                // TODO: (Ian) Get rid of magic numbers
                int w = 60;
                int h = 100;
                // For now, just construct the rectangle to be the same as the player
                int x = packetvi.vals[4] - 60 / 2;
                int y = packetvi.vals[5] - 100;
                // TODO: (Ian) Calculate attacks in some other function maybe
                // TODO: (Ian) Create attack class and store attacks in a queue
                for (const ClientUnit& unit : clients) {
                    if (unit.m_id == packetvi.vals[1]) continue;
                    // Check for collisions to see if any damage is dealt
                    int u_x = unit.m_x - 60 / 2;
                    int u_y = unit.m_y - 100;
                    if (x < u_x + 60 && x + w > u_x && y < u_y + 100 && y + h > u_y) {
                        collisions.push_back(unit.m_id);
                    }
                }
                // Then, send all updates to the players
                for (const ClientUnit& unit : clients) {
                    if (unit.m_id != packetvi.vals[1]) network.sendPacketGuarantee(packet.data, unit.m_address);
                    for (int unit_id : collisions) {
                        Socket::Packet2i collision;
                        collision.first = PACKET_UNIT_DAMAGED;
                        collision.second = unit_id;
                        network.sendPacketGuarantee(Socket::createBasicPacket(collision), unit.m_address);
                    }
                }
            }
        }
    }
}

void Instance::addNewClient(Socket::Address address) {
    // Generate a new player ID and give it to the player
    // TODO: (Ian) Make sure the ID doesn't already exist
    int clientId = rand() % 10000;
    clients.push_back({clientId, address, 0, 0, 5});
    Socket::Packet1i response = { clientId };
    Socket::BasicPacket con_response = Socket::createBasicPacket(response);
    network.sendPacketGuarantee(con_response, address);
    LOG("Accepted client connection; client ID: " << clientId);
}

void Instance::clientSender() {

    while (true) {
        // create the client packet
        std::vector<Socket::Packetvi> packets;
        Socket::Packetvi current;
        int current_size = 0;
        current.vals.push_back(PACKET_PLAYER_POS);
        for (const ClientUnit& client : clients) {
            // Start working on a new packet if the current one is filled
            if (current_size > 5) {
                packets.push_back(current);
                current.vals.clear();
                current_size = 0;
            }
            current.vals.push_back(client.m_id);
            current.vals.push_back(client.m_x);
            current.vals.push_back(client.m_y);
        }
        packets.push_back(current);
        for (const auto& packet : packets) {
            Socket::BasicPacket con_packet = Socket::createBasicPacket(packet);
            for (const ClientUnit& client : clients) {
                network.sendPacket(con_packet, client.m_address);
            }
        }
        // TODO: (Ian) Use a delta time calculation to determine send frequency
        std::this_thread::sleep_for(std::chrono::milliseconds(PLAYER_POS_SEND_FREQUENCY));
    }

}