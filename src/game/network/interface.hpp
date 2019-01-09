#pragma once

#include "socket.hpp"
#include "common.hpp"

#include <mutex>
#include <queue>
#include <set>
#include <thread>
#include <atomic>
#include <chrono>

// A wrapper class to include address with each packet to send
struct PacketWrapper {
	Socket::BasicPacket packet;
	Socket::Address address;
	PacketWrapper(Socket::BasicPacket packet, Socket::Address address)
		: packet(packet), address(address) {}
};

// A wrapper class for each guarantee packet with an expected response
struct ExpectedPacket {
	// The expected ID of the packet
	int packet_id;
	// The time that the packet was sent
	std::chrono::milliseconds timestamp;
	// The packet data in case it needs resending
	Socket::BasicPacket data;
	ExpectedPacket(int id) : packet_id(id) {}
	ExpectedPacket(int id, Socket::BasicPacket data) : packet_id(id), data(data) {
		timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	}
};

inline bool operator<(const ExpectedPacket& first, const ExpectedPacket& second) {
	return first.packet_id < second.packet_id;
}

inline bool operator==(const ExpectedPacket& first, const ExpectedPacket& second) {
	return first.packet_id == second.packet_id;
}

/*
 *  The interface class that handles incoming/outgoing packets
 */
class Interface {

public:

	Interface(int port = Socket::DEFAULT_PORT);
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

	// The central packed id that is incremented on each packet
	int    m_packet_id;

	std::mutex                  m_response_lock;
	std::set<ExpectedPacket>    m_expected_response;

	// The function that continuously sends packets
	std::atomic<bool>   m_running;
	std::thread         t_packet_sender;
	void f_packet_sender();

};