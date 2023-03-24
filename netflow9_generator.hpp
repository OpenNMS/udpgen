#ifndef _netflow9_generator_h_
#define _netflow9_generator_h_

#include <random>
extern "C"
{
#include "netflow9.h"
}

#include "udp_generator.hpp"
#include "service.h"

class Netflow9Generator : public UDPGenerator {
public:
    Netflow9Generator();
    ~Netflow9Generator();
    virtual int start();
    virtual void stop();
    virtual const char* getPacketDescription();
    virtual void sendPackets(int threadid, unsigned int num_packets, unsigned long long first_seq);
private:
    void init_template();
    int generate_flow(u_char * packet, u_int len, const struct timeval *system_boot_time, u_int * len_used);

    sockaddr_storage m_dest;
    int* m_sockets = nullptr;

    std::mt19937 m_random_gen;
    std::uniform_int_distribution<uint32_t> m_random_dis;

    struct NF9_SOFTFLOWD_TEMPLATE m_v4_template;
    struct NF9_SOFTFLOWD_TEMPLATE m_v6_template;
    struct timeval m_system_boot_time;
};

#endif /* _netflow9_generator_h_ */
