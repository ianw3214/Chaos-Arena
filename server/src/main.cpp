#include "socket.hpp"
#include "common.hpp"
#include "util.hpp"

#include "interface.hpp"
#include "game/instance.hpp"

#include <ctime>

// TODO: Some kind of queue for sending messages so that multiple threads can access at the same time
int main(int argc, char* argv[]) {

    srand(time(NULL));

    // INITIALIZE THE SOCKET API
    if (!Socket::init()) return 1;

    // Instantiate an interface for the network
    Interface network;
    Instance instance(network);

    // RECEIVING A PACKET - 'receive'
    while (true) {
        Socket::Packet<Socket::BasicPacket> packet = network.recieve();
        instance.packetRecieved(packet);
    }

    Socket::shutdown();

    return 0;
}