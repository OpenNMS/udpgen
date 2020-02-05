#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/time.h>

#include "netflow5_generator.hpp"
extern "C"
{
#include "netflow5.h"
}

Netflow5Generator::Netflow5Generator() {
    setPort(8877);
}

Netflow5Generator::~Netflow5Generator() = default;

int Netflow5Generator::start() {
    char service[16];
    snprintf(service, 16, "%d", getPort());
    if (resolvehelper(getHost(), AF_INET, service, &m_dest)) {
        perror("resolve()");
        return -1;
    }

    m_sockets = new int[getNumThreads()];
    for (int i = 0; i < getNumThreads(); ++i) {
        m_sockets[i] = socket(AF_INET, SOCK_DGRAM, 0);
        if (connect(m_sockets[i], (sockaddr *)&m_dest, sizeof(m_dest)) < 0) {
            perror("connect()");
            return -1;
        }
    }

    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    m_random_gen = std::mt19937(rd()); //Standard mersenne_twister_engine seeded with rd()
    m_random_dis = std::uniform_int_distribution<uint32_t>();

    return UDPGenerator::start();
}

void Netflow5Generator::stop() {
    UDPGenerator::stop();

    if (m_sockets != nullptr) {
        for (int i = 0; i < getNumThreads(); ++i) {
            close(m_sockets[i]);
            m_sockets[i] = -1;
        }
        delete[] m_sockets;
        m_sockets = nullptr;
    }
}

const char* Netflow5Generator::getPacketDescription() {
    return "Netflow 5 Flows";
}

void Netflow5Generator::sendPackets(int threadid, unsigned int num_packets, unsigned long long first_seq) {
    struct timeval now;
    u_int8_t packet[NF5_MAXPACKET_SIZE];	/* Maximum allowed packet size (30 flows) */
    struct NF5_HEADER *hdr = nullptr;
    struct NF5_FLOW *flw = nullptr;
    int offset;
    int num_flows = 30;

    gettimeofday(&now, NULL);

    memset(&packet, '\0', sizeof(packet));
    hdr = (struct NF5_HEADER *)packet;

    hdr->version = htons(5);
    hdr->flows = htons(num_flows);
    hdr->uptime_ms = htonl(first_seq);
    hdr->time_sec = htonl(now.tv_sec);
    hdr->time_nanosec = htonl(now.tv_usec * 1000);
    hdr->flow_sequence = htonl(first_seq);

    offset = sizeof(*hdr);

    for (uint32_t i = 0; i < num_flows; i++) {
        flw = (struct NF5_FLOW *)(packet + offset);

        flw->if_index_in = flw->if_index_out = htons(i);
        flw->src_ip = m_random_dis(m_random_gen);
        flw->dest_ip = m_random_dis(m_random_gen);
        flw->src_port = htons(1);
        flw->dest_port = htons(1);
        flw->flow_packets = htonl(1);
        flw->flow_octets = htonl(1);
        flw->flow_start = htonl(0);
        flw->flow_finish = htonl(1);
        flw->tcp_flags = 0;
        flw->protocol = 0;
        flw->tos = 0;
        offset += sizeof(*flw);
    }

    send(m_sockets[threadid], packet, (size_t)offset, 0);
}
