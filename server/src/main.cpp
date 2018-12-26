#include "socket.hpp"
#include "common.hpp"
#include "util.hpp"

#include "instance.hpp"

#include <ctime>

// TODO: Some kind of queue for sending messages so that multiple threads can access at the same time
int main(int argc, char* argv[]) {

    srand(time(NULL));

    // INITIALIZE THE SOCKET API
    if (!Socket::init()) return 1;

    // Instantiate a socket and bind it
    Socket::Socket socket = Socket::create();
    if (socket <= 0) ERR("Could not create socket...");
    Socket::bind(socket);

    Instance instance(socket);

    // RECEIVING A PACKET - 'receive'
    while (true) {
        Socket::Packet<Socket::BasicPacket> packet = Socket::recieve<Socket::BasicPacket>(socket);
        if (packet.has_data) {
            instance.packetRecieved(packet);
        }
    }

    Socket::close(socket);

    Socket::shutdown();

    return 0;
}