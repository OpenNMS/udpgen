#ifndef _netflow9_generator_h_
#define _netflow9_generator_h_

#include <random>
extern "C"
{
#include "netflow9.h"
}

#include "udp_generator.hpp"

struct Service {
  int port;
  int protocol;
};

// a list of common network service
const Service services[] = {
  {20, 6},
  {21, 6},
  {22, 6},
  {23, 6},
  {25, 6},
  {53, 6},
  {67, 17},
  {68, 17},
  {80, 6},
  {110, 6},
  {123, 17},
  {143, 6},
  {161, 17},
  {162, 17},
  {179, 6},
  {201, 6},
  {389, 6},
  {443, 6},
  {500, 17},
  {514, 17},
  {587, 6},
  {993, 6},
  {1701, 17},
  {3389, 6},
  {3306, 6},
  {4500, 17},
  {0, 1},
  {0, 50},
  {0, 51}
};

const int services_size = sizeof(services)/sizeof(Service);

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
