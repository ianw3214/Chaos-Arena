#pragma once

namespace Socket {

    class Address {
        public:
            Address() : address(0), port(0) {

            }
            Address(unsigned char a, 
                unsigned char b, 
                unsigned char c, 
                unsigned char d, 
                unsigned short port) :
                address((a << 24) | (b << 16) | (c << 8) | d),
                port(port) {}
            Address(unsigned int address, unsigned short port)
                : address(address), port(port) {}

            // Getter methods
            unsigned int getAddress() const { return address; }
            unsigned char getA() const      { return address >> 24; }
            unsigned char getB() const      { return address << 8 >> 24; }
            unsigned char getC() const      { return address << 16 >> 24; }
            unsigned char getD() const      { return address << 24 >> 24; }
            unsigned short getPort() const  { return port; }

            // Utility operator methods
            bool operator==(const Address& other) const {
                return address == other.address && port == other.port;
            }
            bool operator<(const Address& other) const {
                return address == other.address ? port < other.port : address < other.address;
            }
        private:
            unsigned int address;
            unsigned short port;
    };
    
}