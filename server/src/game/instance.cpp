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
        if (packet2i.first == PACKET_UNIT_RESPAWN) {
            // Also let the units health be restored to normal
            ClientUnit * client = getClientById(packet2i.second);
            if (client) client->m_health = PLAYER_DEFAULT_HEALTH;
            // Let all clients know that a unit has respawned
            Socket::Packetvi spawn_response;
            spawn_response.vals.push_back(PACKET_UNIT_RESPAWN);
            spawn_response.vals.push_back(packet2i.second);
            spawn_response.vals.push_back(map.getRandomSpawnX());
            spawn_response.vals.push_back(map.getRandomSpawnY());
            for (const ClientUnit& client : clients) {
                network.sendPacketGuarantee(spawn_response, client.m_address);
            }
        }
    }
    if (Socket::getPacketType(packet.data) == PACKET_3I) {
        // Update the corresponding player position
        Socket::Packet3i packet3i = Socket::convertPacket3i(packet.data);
        if (packet3i.first == PACKET_PACKETS_RECIEVED) {
            if (packet3i.second == map.numPackets()) {
                // Send a ready packet if the player has recieved all the packets
                Socket::Packet1i ready_packet = { PACKET_DUNGEON_READY };
                network.sendPacketGuarantee(ready_packet, packet.address);
                ClientUnit * client = getClientById(packet3i.third);
                if (client) {
                    client->ready = true;
                }
            }
        }
        if (packet3i.first == PACKET_PLAYER_DASH) {
            // Update the dashing property of the player
            ClientUnit * client = getClientById(packet3i.second);
            if (client) {
                client->dashing = true;
                client->dash_start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
            }
            // Just forward the packet to the other players
            for (const ClientUnit& client : clients) {
                network.sendPacketGuarantee(packet.data, client.m_address);
            }
        }
    }
    if (Socket::getPacketType(packet.data) == PACKET_VI) {
        Socket::Packetvi packetvi = Socket::convertPacketvi(packet.data);
        // TODO: (Ian) Better erorr handling
        if (packetvi.vals.size() == 0) return;
        if (packetvi.vals[0] == PACKET_PLAYER_POS) {
            int id = packetvi.vals[1];
            int x = packetvi.vals[2];
            int y = packetvi.vals[3];
            ClientUnit * client = getClientById(id);
            if (client) {
                client->m_x = x;
                client->m_y = y;
            }
        }
        if (packetvi.vals[0] == PACKET_PLAYER_ATTACK) {
            if (packetvi.vals[2] == ATTACK_BASIC_PUNCH) {
                if (packetvi.vals.size() < 6) return;
                std::vector<int> collisions;
                int x = packetvi.vals[4] - PLAYER_WIDTH / 2;
                if (packetvi.vals[3] == FACE_RIGHT) {
                    LOG("FACE RIGHT PUNCH");
                    x += PUNCH_OFFSET_X;
                } else {
                    LOG("FACE LEFT PUNCH");
                    x -= PUNCH_OFFSET_X - PLAYER_WIDTH + PUNCH_WIDTH;
                }
                int y = packetvi.vals[5] - PLAYER_HEIGHT + PUNCH_OFFSET_Y;
                int w = PUNCH_WIDTH;
                int h = PUNCH_HEIGHT;
                // TODO: (Ian) Calculate attacks in some other function maybe
                // TODO: (Ian) Create attack class and store attacks in a queue
                for (const ClientUnit& unit : clients) {
                    if (unit.m_id == packetvi.vals[1]) continue;
                    // If the unit is dead, ignore it
                    if (unit.m_health <= 0) continue;
                    // Check for collisions to see if any damage is dealt
                    int u_x = unit.m_x - PLAYER_WIDTH / 2;
                    int u_y = unit.m_y - PLAYER_HEIGHT;
                    if (x < u_x + 60 && x + w > u_x && y < u_y + 100 && y + h > u_y) {
                        collisions.push_back(unit.m_id);
                    }
                }
                // First broadcast the attack data back to the players
                for (const ClientUnit& unit : clients) {
                    if (unit.m_id != packetvi.vals[1]) network.sendPacketGuarantee(packet.data, unit.m_address);
                }
                // Update all units hit by the attack
                for (int unit_id : collisions) {
                    // Remove a health from the player
                    ClientUnit * client = getClientById(unit_id);
                    if (!client) return;    // ERROR
                    bool unit_dead = false;
                    // Ignore the collision if the unit is dashing
                    bool take_damage = true;
                    if (client->dashing) {
                        std::chrono::milliseconds timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
                        int time_elapsed = (timestamp - client->dash_start).count();
                        if (time_elapsed > PLAYER_DASH_TIME) {
                            client->dashing = false;
                        } else {
                            take_damage = false;
                        }
                    }
                    // Then handle when the units take damage
                    if (take_damage) {
                        // Damage the unit and send a kill packet if it dies
                        if (--(client->m_health) <= 0) {
                            unit_dead = true;
                            // If a unit died, give the player a kill
                            Socket::Packet1i kill_packet;
                            kill_packet.val = PACKET_PLAYER_KILL;
                            network.sendPacketGuarantee(kill_packet, packet.address);
                        }
                        // Send the damaged packet to all clients
                        for (const ClientUnit& unit : clients) {
                            // Send packets based on whether the unit died or not
                            if (unit_dead) {
                                Socket::Packet2i death_packet;
                                death_packet.first = PACKET_UNIT_DEAD;
                                death_packet.second = unit_id;
                                network.sendPacketGuarantee(Socket::createBasicPacket(death_packet), unit.m_address);
                            } else {
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
    }
}

void Instance::addNewClient(Socket::Address address) {
    // Generate a new player ID and give it to the player
    int clientId = rand() % 10000;
    while(getClientById(clientId)) {
        clientId = rand() % 10000;
    }
    clients.push_back({clientId, address, false, 0, 0, PLAYER_DEFAULT_HEALTH, false});
    Socket::Packet1i response = { clientId };
    Socket::BasicPacket con_response = Socket::createBasicPacket(response);
    network.sendPacketGuarantee(con_response, address);
    LOG("Accepted client connection; client ID: " << clientId);
}

ClientUnit * Instance::getClientById(int id) {
    for (ClientUnit& client : clients) {
        if (client.m_id == id) {
            return &client;
        }
    }
    return nullptr;
}

void Instance::clientSender() {

    while (true) {
        // create the client packet
        std::vector<Socket::Packetvi> packets;
        Socket::Packetvi current;
        int current_size = 0;
        current.vals.push_back(PACKET_PLAYER_POS);
        for (const ClientUnit& client : clients) {
            // If the client is not ready, skip it
            if (!client.ready) continue;
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