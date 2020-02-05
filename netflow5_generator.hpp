#ifndef _netflow5_generator_h_
#define _netflow5_generator_h_

#include <random>

#include "udp_generator.hpp"

class Netflow5Generator : public UDPGenerator {
public:
    Netflow5Generator();
    ~Netflow5Generator();
    virtual int start();
    virtual void stop();
    virtual const char* getPacketDescription();
    virtual void sendPackets(int threadid, unsigned int num_packets, unsigned long long first_seq);
private:
    sockaddr_storage m_dest;
    int* m_sockets = nullptr;

    std::mt19937 m_random_gen;
    std::uniform_int_distribution<uint32_t> m_random_dis;
};

#endif /* _netflow5_generator_h_ */