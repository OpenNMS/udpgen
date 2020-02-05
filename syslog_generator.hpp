#ifndef _syslog_generator_h_
#define _syslog_generator_h_

#include "udp_generator.hpp"

class SyslogGenerator : public UDPGenerator {
public:
    SyslogGenerator();
    virtual int start();
    virtual void stop();
    virtual const char* getPacketDescription();
    virtual void sendPackets(int threadid, unsigned int num_packets, unsigned long long first_seq);
private:
    sockaddr_storage m_dest;

    static const int SEQ_BUFFER_SIZE = 64;

    int* m_sockets = NULL;
    struct msghdr* m_message_headers = NULL;
    struct iovec** m_message_iovs = NULL;
    char** m_seq_buffers = NULL;
};

#endif