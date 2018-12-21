#pragma once

// PLATFORM DETECTION
#define PLATFORM_WINDOWS    1
#define PLATFORM_MAC        2
#define PLATFORM_UNIX       3

#if defined(_WIN32)
#define PLATFORM PLATFORM_WINDOWS
#elif defined (__APPLE__)
#define PLATFORM PLATFORM_MAC
#else
#define PLATFORM PLATFORM_UNIX
#endif

// INCLUDE FILES BASED ON THE OPERATING SYSTEM
#if PLATFORM == PLATFORM_WINDOWS
#include <winsock2.h>
typedef int socklen_t;
#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#endif

#include "util.hpp"

namespace Socket {

    // SOME DEFINITIONS
    using Socket = int;
    const int DEFAULT_PORT = 2000;

    // SOCKET INITIALIZATION
    inline bool init() {
        #if PLATFORM == PLATFORM_WINDOWS
        WSADATA WsaData;
        return WSAStartup( MAKEWORD(2, 2), &WsaData) == NO_ERROR;
        #else
        return true;
        #endif
    }
    inline void shutdown() {
        #if PLATFORM == PLATFORM_WINDOWS
        WSACleanup();
        #endif
    }

    // SOCKET FUNCTIONS
    inline Socket create() {
        return socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }
    inline bool bind(Socket socket, int port = DEFAULT_PORT) {
        sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(static_cast<unsigned short>(port));
        if (bind(socket, (const sockaddr*) &address, sizeof(sockaddr_in)) < 0) return false;
        return true;
    }
    inline void close(Socket socket) {
        #if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
        close(socket);
        #elif PLATFORM == PLATFORM_WINDOWS
        closesocket(socket);
        #endif
    }
    inline bool setNonBlock(Socket socket) {
        #if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
        int nonBlocking = 1;
        if (fcntl(socket, F_SETFL, O_NONBLOCK, nonBlocking) == -1) return false;
        #elif PLATFORM == PLATFORM_WINDOWS
        DWORD nonBlocking = 1;
        if (ioctlsocket(socket, FIONBIO, &nonBlocking) != 0) return false;
        #endif
        return true;
    }

    /*  -------------------------------------------------------------------------
            TEMPLATED SOCKET SEND/RECEIVE FUNCTIONS
        ------------------------------------------------------------------------- */

    template<class T>
    bool send(Socket socket, Address address, T packet) {
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(address.getAddress());
        addr.sin_port = htons(address.getPort());
        if (sendto(socket, (const char*) &packet, sizeof(T), 0, (sockaddr*) &addr, sizeof(sockaddr_in)) < 0) {
            // Error handling here
            return false;
        }
        // Handle success here
        return true;
    }

    // Wrapper to help determine if any data was recieved
    template<class T>
    struct Packet {
        bool has_data;
        Address address;
        T data;
    };

    template<class T>
    Packet<T> recieve(Socket socket) {
        Packet<T> packet;
        packet.has_data = false;
        sockaddr_in from;
        socklen_t fromLength = sizeof(from);
        int bytes = recvfrom(socket, (char*)&(packet.data), sizeof(T), 0, (sockaddr*)&from, &fromLength);
        // recvfrom(socket, (char*)&packet, sizeof(T), 0, (sockaddr*)&from, &fromLength);
        if (bytes >= 0) {
            packet.has_data = true;
            packet.address = Address(ntohl(from.sin_addr.s_addr), ntohs(from.sin_port));
        }
        return packet;
    }

}