#ifndef _udp_generator_h_
#define _udp_generator_h_

#include <mutex>
#include <sys/socket.h>
#include <thread>
#include <atomic>
#include "rate_limiter_interface.hpp"

class UDPGenerator {
public:
    UDPGenerator();
    virtual ~UDPGenerator();

    void setHost(const char* host);
    const char* getHost();

    void setPort(int port);
    int getPort();

    void setNumThreads(int num_threads);
    int getNumThreads();

    void setPacketsPerSecond(double packets_per_second);
    double getPacketsPerSecond();

    void setReportInterval(int report_interval);
    int getReportInterval();

    void setNumPacketsPerSend(unsigned int num_packets);
    unsigned int getNumPacketsPerSend();

    virtual int start();
    virtual void stop();

    void runWithRateLimit(int threadid);
    void runWithoutRateLimit(int threadid);

    virtual const char* getPacketDescription() = 0;
    virtual void sendPackets(int threadid, unsigned int num_packets, unsigned long long first_seq) = 0;

    int resolvehelper(const char *hostname, int family, const char *service, sockaddr_storage *pAddr);

private:
    const char* m_host = "127.0.0.1";
    int m_port = 0;
    int m_num_threads = 1;
    double m_packets_per_second = 10000;
    int m_report_interval = 10;
    unsigned int m_num_packets_per_send = 1;

    unsigned long long m_report_every_n_packets;
    bool m_stopped;

    std::unique_ptr<std::thread[]> m_threads;
    std::unique_ptr<RateLimiterInterface> m_limiter;
    std::atomic_ullong m_sequence_counter;
};

#endif