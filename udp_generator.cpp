#include "udp_generator.hpp"

#include <iostream>
#include <netdb.h>
#include <cstring>
#include <algorithm>
#include "rate_limiter.hpp"

UDPGenerator::UDPGenerator() {

}

UDPGenerator::~UDPGenerator() {
    stop();
}

void UDPGenerator::setHost(const char* host) {
    m_host = host;
}

const char* UDPGenerator::getHost() {
    return m_host;
}

void UDPGenerator::setPort(int port) {
    m_port = port;
}

int UDPGenerator::getPort() {
    return m_port;
}

void UDPGenerator::setNumThreads(int num_threads) {
    m_num_threads = num_threads;
}

int UDPGenerator::getNumThreads() {
    return m_num_threads;
}

void UDPGenerator::setPacketsPerSecond(double packets_per_second) {
    m_packets_per_second = packets_per_second;
}

double UDPGenerator::getPacketsPerSecond() {
    return m_packets_per_second;
}

void UDPGenerator::setReportInterval(int report_interval) {
    m_report_interval = report_interval;
}

int UDPGenerator::getReportInterval() {
    return m_report_interval;
}

void UDPGenerator::setNumPacketsPerSend(unsigned int num_packets_per_send) {
    m_num_packets_per_send = num_packets_per_send;
}

unsigned int UDPGenerator::getNumPacketsPerSend() {
    return m_num_packets_per_send;
}

int UDPGenerator::resolvehelper(const char *hostname, int family, const char *service, sockaddr_storage *pAddr) {
    int result;
    addrinfo *result_list = NULL;
    addrinfo hints = {};
    hints.ai_family = family;
    hints.ai_socktype = SOCK_DGRAM; // without this flag, getaddrinfo will return 3x the number of addresses (one for each socket type).
    result = getaddrinfo(hostname, service, &hints, &result_list);
    if (result == 0) {
        memcpy(pAddr, result_list->ai_addr, result_list->ai_addrlen);
        freeaddrinfo(result_list);
    }
    return result;
}

int UDPGenerator::start() {
    if (m_packets_per_second > 0) {
        m_limiter = std::unique_ptr<RateLimiter>(new RateLimiter());
        m_limiter.get()->set_rate(m_packets_per_second);
    }

    m_sequence_counter.store(0);
    m_stopped = false;

    m_threads = std::unique_ptr<std::thread[]>(new std::thread[m_num_threads]);
    for (int i = 0; i < m_num_threads; ++i) {
        if (m_packets_per_second > 0) {
            m_report_every_n_packets = (unsigned long)(m_packets_per_second) * m_report_interval;
            m_threads[i] = std::thread(&UDPGenerator::runWithRateLimit, this, i);
        } else {
            m_report_every_n_packets = (unsigned long)1000000 * m_report_interval;
            m_threads[i] = std::thread(&UDPGenerator::runWithoutRateLimit, this, i);
        }
    }
    return 0;
}

void UDPGenerator::runWithRateLimit(int threadid) {
    time_t now;
    const int DATE_BUFF_SIZE = 100;
    char date_buff[DATE_BUFF_SIZE];

    while(!m_stopped) {
        m_limiter.get()->aquire(m_num_packets_per_send);
        unsigned long long seq = m_sequence_counter.fetch_add(m_num_packets_per_send);
        sendPackets(threadid, m_num_packets_per_send, seq - m_num_packets_per_send);
        if (seq % m_report_every_n_packets == 0) {
            now = time(0);
            strftime(date_buff, DATE_BUFF_SIZE, "%Y-%m-%d %H:%M:%S.000", localtime(&now));
            printf ("%s: Sent %llu packets.\n", date_buff, seq);
        }
    }
}

void UDPGenerator::runWithoutRateLimit(int threadid) {
    time_t now;
    const int DATE_BUFF_SIZE = 100;
    char date_buff[DATE_BUFF_SIZE];

    while(!m_stopped) {
        unsigned long long seq = m_sequence_counter.fetch_add(m_num_packets_per_send);
        sendPackets(threadid, m_num_packets_per_send, seq - m_num_packets_per_send);
        if (seq % m_report_every_n_packets == 0) {
            now = time(0);
            strftime(date_buff, DATE_BUFF_SIZE, "%Y-%m-%d %H:%M:%S.000", localtime(&now));
            printf ("%s: Sent %llu packets.\n", date_buff, seq);
        }
    }
}

void UDPGenerator::stop() {
    if (!m_stopped) {
        m_stopped = true;
        std::cout << "Stopping..." << std::endl;
        for (int i = 0; i < m_num_threads; ++i) {
            m_threads[i].join();
        }
    }
}
