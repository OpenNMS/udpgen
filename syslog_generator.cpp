#include "syslog_generator.hpp"
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <cstring>
#include <unistd.h>

SyslogGenerator::SyslogGenerator() {
    setPort(1514);
}

int SyslogGenerator::start() {
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

    m_message_headers = new struct msghdr[getNumThreads()];
    m_message_iovs = new struct iovec*[getNumThreads()];
    m_seq_buffers = new char*[getNumThreads()];
    for (int i = 0; i < getNumThreads(); ++i) {
        memset(&m_message_headers[i], 0, sizeof(struct msghdr));

        // Split the message up 3 vectors
        // 1) The prefix - constant
        // 2) The sequence ID - incremented on every packet
        // 3) The suffix - constant
        m_message_iovs[i] = new iovec[3];
        m_message_iovs[i][0].iov_base = (void*)"<190>Mar 11 08:35:17 fw01 ";
        m_message_iovs[i][0].iov_len = strlen((const char*)m_message_iovs[i][0].iov_base);

        m_seq_buffers[i] = new char[SEQ_BUFFER_SIZE];
        memset(m_seq_buffers[i], 0, SEQ_BUFFER_SIZE);
        m_message_iovs[i][1].iov_base = (void*)m_seq_buffers[i];
        m_message_iovs[i][1].iov_len = 0;

        m_message_iovs[i][2].iov_base = (void*)": Mar 11 08:35:16.844 CST: %%SEC-6-IPACCESSLOGP: list in110 denied tcp 10.99.99.1(63923) -> 10.98.98.1(1521), 1 packet";
        m_message_iovs[i][2].iov_len = strlen((const char*)m_message_iovs[i][2].iov_base);

        m_message_headers[i].msg_iov = m_message_iovs[i];
        m_message_headers[i].msg_iovlen = 3;
    }

    return UDPGenerator::start();
}

void SyslogGenerator::stop() {
    UDPGenerator::stop();

    if (m_sockets != NULL) {
        for (int i = 0; i < getNumThreads(); ++i) {
            close(m_sockets[i]);
            m_sockets[i] = -1;
        }
        delete[] m_sockets;
        m_sockets = NULL;
    }

    if (m_message_headers != NULL) {
        delete[] m_message_headers;
        m_message_headers = NULL;
    }

    if (m_message_iovs != NULL) {
        for (int i = 0; i < getNumThreads(); ++i) {
            delete[] m_message_iovs[i];
        }
        delete[] m_message_iovs;
        m_message_iovs = NULL;
    }

    if (m_seq_buffers != NULL) {
        for (int i = 0; i < getNumThreads(); ++i) {
            delete[] m_seq_buffers[i];
        }
        delete[] m_seq_buffers;
        m_seq_buffers = NULL;
    }
}

const char* SyslogGenerator::getPacketDescription() {
    return "Syslog Messages";
}

void SyslogGenerator::sendPackets(int threadid, unsigned int num_packets, unsigned long long first_seq) {
    struct msghdr *message_header = &m_message_headers[threadid];
    struct iovec *seq_iovec = &m_message_iovs[threadid][1];
    char *seq_buffer = m_seq_buffers[threadid];

    for (unsigned int i = 0; i < num_packets; i++) {
        // Update the sequence buffer with the current sequence id
        seq_iovec->iov_len = (size_t) snprintf(seq_buffer, SEQ_BUFFER_SIZE, "%llu", first_seq + i);

        sendmsg(m_sockets[threadid], message_header, 0);
        /* This throws an error, but the packets are successfully sent ?!
        if (ret < 0) {
            perror("sendmsg()");
        }*/
    }
}
