#include "interface.hpp"

Interface::Interface(int port) : m_port(port) {
	m_socket = Socket::create();
	Socket::bind(m_socket, m_port);

	// Don't use 0 because that means NOT guaranteed
	m_packet_id = 1;

	m_running = true;
	t_packet_sender = std::thread(&Interface::f_packet_sender, this);
}

Interface::~Interface() {
	m_running = false;
	t_packet_sender.join();

	Socket::close(m_socket);

}

void Interface::setNonBlock() {
	Socket::setNonBlock(m_socket);
}

void Interface::sendPacket(Socket::Packet1i packet, Socket::Address address) {
	m_send_lock.lock();
	packets.emplace(Socket::createBasicPacket(packet), address);
	m_send_lock.unlock();
}

void Interface::sendPacket(Socket::Packet2i packet, Socket::Address address) {
	m_send_lock.lock();
	packets.emplace(Socket::createBasicPacket(packet), address);
	m_send_lock.unlock();
}

void Interface::sendPacket(Socket::Packet3i packet, Socket::Address address) {
	m_send_lock.lock();
	packets.emplace(Socket::createBasicPacket(packet), address);
	m_send_lock.unlock();
}

void Interface::sendPacket(Socket::Packetvi packet, Socket::Address address) {
	m_send_lock.lock();
	packets.emplace(Socket::createBasicPacket(packet), address);
	m_send_lock.unlock();
}

void Interface::sendPacket(Socket::BasicPacket packet, Socket::Address address) {
	packet.packet_id = 0;
	m_send_lock.lock();
	packets.emplace(packet, address);
	m_send_lock.unlock();
}

void Interface::sendPacketGuarantee(Socket::Packet1i packet, Socket::Address address) {
	m_send_lock.lock();
	int id = m_packet_id++;
	if (m_packet_id == 0) m_packet_id = 1;
	packets.emplace(Socket::createBasicPacket(packet, id), address);
	m_send_lock.unlock();
	m_response_lock.lock();
	m_expected_response.emplace(id, Socket::createBasicPacket(packet, id));
	m_response_lock.unlock();
}

void Interface::sendPacketGuarantee(Socket::Packet2i packet, Socket::Address address) {
	m_send_lock.lock();
	int id = m_packet_id++;
	if (m_packet_id == 0) m_packet_id = 1;
	packets.emplace(Socket::createBasicPacket(packet, id), address);
	m_send_lock.unlock();
	m_response_lock.lock();
	m_expected_response.emplace(id, Socket::createBasicPacket(packet, id));
	m_response_lock.unlock();
}
void Interface::sendPacketGuarantee(Socket::Packet3i packet, Socket::Address address) {
	m_send_lock.lock();
	int id = m_packet_id++;
	if (m_packet_id == 0) m_packet_id = 1;
	packets.emplace(Socket::createBasicPacket(packet, id), address);
	m_send_lock.unlock();
	m_response_lock.lock();
	m_expected_response.emplace(id, Socket::createBasicPacket(packet, id));
	m_response_lock.unlock();
}
void Interface::sendPacketGuarantee(Socket::Packetvi packet, Socket::Address address) {
	m_send_lock.lock();
	int id = m_packet_id++;
	if (m_packet_id == 0) m_packet_id = 1;
	packets.emplace(Socket::createBasicPacket(packet, id), address);
	m_send_lock.unlock();
	m_response_lock.lock();
	m_expected_response.emplace(id, Socket::createBasicPacket(packet, id));
	m_response_lock.unlock();
}

void Interface::sendPacketGuarantee(Socket::BasicPacket packet, Socket::Address address) {
	m_send_lock.lock();
	int id = m_packet_id++;
	if (m_packet_id == 0) m_packet_id = 1;
	packet.packet_id = id;
	packets.emplace(packet, address);
	m_send_lock.unlock();
	m_response_lock.lock();
	m_expected_response.emplace(id, packet);
	m_response_lock.unlock();
}

Socket::Packet<Socket::BasicPacket> Interface::recieve() {
	Socket::Packet<Socket::BasicPacket> packet = Socket::recieve<Socket::BasicPacket>(m_socket);
	// If the recieved packet is a guaranteed packet, send the response
	if (packet.has_data && Socket::isPacketGuaranteed(packet.data)) {
		Socket::Packet2i response;
		response.first = PACKET_ID_RESPONSE;
		response.second = Socket::getPacketId(packet.data);
		sendPacket(response, packet.address);
	}
	// If the packet id is in the expected response, remove it

	return packet;
}

// The function that continuously sends packets
void Interface::f_packet_sender() {
	while (m_running) {
		if (!packets.empty()) {
			m_send_lock.lock();
			PacketWrapper packet = packets.front();
			packets.pop();
			m_send_lock.unlock();
			Socket::send(m_socket, packet.address, packet.packet);
		}
	}
}