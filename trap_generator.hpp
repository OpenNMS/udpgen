#ifndef _trap_generator_h_
#define _trap_generator_h_

#include "udp_generator.hpp"

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

class TrapGenerator : public UDPGenerator {
public:
    TrapGenerator();
    ~TrapGenerator();
    virtual int start();
    virtual void stop();
    virtual const char* getPacketDescription();
    virtual void sendPackets(int threadid, unsigned int num_packets, unsigned long long first_seq);
private:
    const char* m_community = "public";

    netsnmp_pdu *m_template_pdu;

    netsnmp_session* m_session;
    std::unique_ptr<netsnmp_session*[]> m_sessions;
};

#endif