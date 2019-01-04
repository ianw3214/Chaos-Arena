#include "instance.hpp"

#include <thread>
#include <chrono>

Instance::Instance(const Socket::Socket& socket) : socket(socket) {

    // Initialize instance properties
    map.generate();

    // Start the thread to send packets from the queue
    std::thread t_packetSender(&Instance::packetSender, this);
    t_packetSender.detach();

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
            // Socket::send(socket, packet.address, con_response);
            queuePacket(con_response, packet.address);
            LOG("Accepted client connection; client ID: " << clientId);
            // Send dungeon generation data to the client
            // TODO: (Ian) Handle packet loss
            auto packets = map.generatePackets();
            for (Socket::BasicPacket& map_packet : packets) {
                queuePacket(map_packet, packet.address);
            }
            Socket::Packet1i ready_packet = { PACKET_DUNGEON_READY };
            queuePacket(Socket::createBasicPacket(ready_packet), packet.address);
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
                queuePacket(Socket::createBasicPacket(packet2i), client.m_address);
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
    if (Socket::getPacketType(packet.data) == PACKET_VI) {
        Socket::Packetvi packetvi = Socket::convertPacketvi(packet.data);
        // TODO: (Ian) Better erorr handling
        if (packetvi.vals.size() == 0) return;
        if (packetvi.vals[0] == PACKET_PLAYER_ATTACK) {
            if (packetvi.vals[2] == ATTACK_BASIC_PUNCH) {
                if (packetvi.vals.size() < 6) return;
                std::vector<int> collisions;
                // TODO: (Ian) Get rid of magic numbers
                int w = 60;
                int h = 100;
                // For now, just construct the rectangle to be the same as the player
                int x = packetvi.vals[4];
                int y = packetvi.vals[5];
                // TODO: (Ian) Calculate attacks in some other function maybe
                // TODO: (Ian) Create attack class and store attacks in a queue
                for (const ClientUnit& unit : clients) {
                    // Check for collisions to see if any damage is dealt
                    int u_x = unit.m_x - 60 / 2;
                    int u_y = unit.m_y - 100;
                    if (x < u_x + 60 && x + w > u_x && y < u_y + 100 && y + h > u_y) {
                        collisions.push_back(unit.m_id);
                        
                    }
                }
                // Then, send all updates to the players
                for (const ClientUnit& unit : clients) {
                    if (unit.m_id != packetvi.vals[1]) queuePacket(packet.data, unit.m_address);
                    for (int unit_id : collisions) {
                        Socket::Packet2i collision;
                        collision.first = PACKET_UNIT_DAMAGED;
                        collision.second = unit_id;
                        queuePacket(Socket::createBasicPacket(collision), unit.m_address);
                    }
                }
            }
        }
    }
}

void Instance::queuePacket(Socket::BasicPacket packet, Socket::Address address) {
    packet_lock.lock();
    packets.emplace(packet, address);
    packet_lock.unlock();
    return;
}

void Instance::clientSender() {

    while (true) {
        // create the client packet
        Socket::Packetvi packet;
        packet.vals.push_back(PACKET_PLAYER_POS);
        for (const ClientUnit& client : clients) {
            packet.vals.push_back(client.m_id);
            packet.vals.push_back(client.m_x);
            packet.vals.push_back(client.m_y);
        }
        Socket::BasicPacket con_packet = Socket::createBasicPacket(packet);
        for (const ClientUnit& client : clients) {
            // Socket::send(socket, client.m_address, con_packet);
            queuePacket(con_packet, client.m_address);
        }
        // TODO: (Ian) Use a delta time calculation to determine send frequency
        std::this_thread::sleep_for(std::chrono::milliseconds(PLAYER_POS_SEND_FREQUENCY));
    }

}

void Instance::packetSender() {
    while (true) {
        if (!packets.empty()) {
            packet_lock.lock();
            QueuePacket packet = packets.front();
            packets.pop();
            packet_lock.unlock();
            Socket::send(socket, packet.address, packet.packet);
        }
    }
}