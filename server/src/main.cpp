#include "socket.hpp"
#include "common.hpp"
#include "util.hpp"

#include "interface.hpp"
#include "game/instance.hpp"

#include <ctime>
#include <thread>
#include <chrono>
#include <iostream>

#define IDLE_THRESHOLD  10000
#define IDLE_INTERVALS  10000
#define TIME            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

// TODO: Some kind of queue for sending messages so that multiple threads can access at the same time
int main(int argc, char* argv[]) {

    srand(time(NULL));

    // INITIALIZE THE SOCKET API
    if (!Socket::init()) return 1;

    // Instantiate an interface for the network
    Interface network;
    network.setNonBlock();
    Instance instance(network);

    std::chrono::milliseconds last_packet;
    bool idling = false;
    last_packet = TIME;

    // RECEIVING A PACKET - 'receive'
    while (true) {
        Socket::Packet<Socket::BasicPacket> packet = network.recieve();
        if (packet.has_data) {
            if (idling) {
                std::cout << "SERVER RESUMING" << std::endl;
            }
            last_packet = TIME;
            idling = false;
            network.unpause();
            instance.packetRecieved(packet);
        } else {
            // Server idling logic
            std::chrono::milliseconds now = TIME;
            if (idling || (now - last_packet).count() > IDLE_THRESHOLD) {
                if (!idling) {
                    std::cout << "SERVER IDLING" << std::endl;
                }
                network.pause();
                idling = true;
                std::this_thread::sleep_for(std::chrono::milliseconds(IDLE_INTERVALS));
            }
        }
    }

    Socket::shutdown();

    return 0;
}