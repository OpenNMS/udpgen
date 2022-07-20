/*
 * This code is largely adapted from softflowd.
 *
 * Copyright 2002 Damien Miller <djm@mindrot.org> All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/time.h>

#include "netflow9_generator.hpp"

Netflow9Generator::Netflow9Generator() {
    setPort(9999);
    init_template();

    gettimeofday (&m_system_boot_time, NULL);
}

Netflow9Generator::~Netflow9Generator() = default;

int Netflow9Generator::start() {
    if (resolvehelper(getHost(), AF_INET, getPort(), &m_dest)) {
        printf("Resolving '%s' failed.\n", getHost());
        return -1;
    }

    m_sockets = new int[getNumThreads()];
    for (int i = 0; i < getNumThreads(); ++i) {
        m_sockets[i] = socket(AF_INET, SOCK_DGRAM, 0);
        if (connect(m_sockets[i], (sockaddr *)&m_dest, sizeof(struct sockaddr)) < 0) {
            perror("connect()");
            return -1;
        }
    }

    std::random_device rd;  // Will be used to obtain a seed for the random number engine
    m_random_gen = std::mt19937(rd()); // Standard mersenne_twister_engine seeded with rd()
    m_random_dis = std::uniform_int_distribution<uint32_t>();

    return UDPGenerator::start();
}

void Netflow9Generator::stop() {
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

const char* Netflow9Generator::getPacketDescription() {
    return "Netflow 9 Flows";
}

/*
 * Subtract two timevals. Returns (t1 - t2) in milliseconds.
 */
static u_int32_t timeval_sub_ms (const struct timeval * t1, const struct timeval * t2) {
    struct timeval res;

    res.tv_sec = t1->tv_sec - t2->tv_sec;
    res.tv_usec = t1->tv_usec - t2->tv_usec;
    if (res.tv_usec < 0) {
        res.tv_usec += 1000000L;
        res.tv_sec--;
    }
    return ((u_int32_t) res.tv_sec * 1000 + (u_int32_t) res.tv_usec / 1000);
}

void Netflow9Generator::sendPackets(int threadid, unsigned int num_flows, unsigned long long first_seq) {
    u_char packet[NF9_SOFTFLOWD_MAX_PACKET_SIZE];
    u_int i, j;
    struct NFLOW9_HEADER *nf9;
    struct NF9_DATA_FLOWSET_HEADER *dh;
    u_int offset, last_valid, num_packets, inc;
    int r;
    struct timeval now;
    gettimeofday (&now, NULL);

    last_valid = num_packets = 0;
    for (j = 0; j < num_flows;) {
        memset (packet, 0, sizeof (packet));
        nf9 = (struct NFLOW9_HEADER *) packet;

        nf9->version = htons (9);
        nf9->flows = 0; /* Filled as we go, htons at end */
        nf9->uptime_ms = htonl (timeval_sub_ms (&now, &m_system_boot_time));
        nf9->export_time = htonl (time (NULL));
        nf9->od_id = 0;
        offset = sizeof (*nf9);

        /* Refresh template headers if we need to */
        if (nf9_pkts_until_template <= 0) {
            memcpy (packet + offset, &v4_template, sizeof (v4_template));
            offset += sizeof (v4_template);
            nf9->flows++;
            memcpy (packet + offset, &v6_template, sizeof (v6_template));
            offset += sizeof (v6_template);
            nf9->flows++;
            nf9_pkts_until_template = NF9_DEFAULT_TEMPLATE_INTERVAL;
        }

        dh = NULL;
        for (i = 0; i + j < num_flows; i++) {
            if (dh == NULL) {
                if (offset + sizeof (*dh) > sizeof (packet)) {
                    /* Mark header is finished */
                    dh = NULL;
                    break;
                }
                dh = (struct NF9_DATA_FLOWSET_HEADER *)
                        (packet + offset);
                dh->c.flowset_id = v4_template.h.template_id;
                last_valid = offset;
                dh->c.length = sizeof (*dh);    /* Filled as we go */
                offset += sizeof (*dh);
            }

            r = generate_flow(packet + offset, sizeof (packet) - offset, &m_system_boot_time, &inc);
            if (r <= 0) {
                /* yank off data header, if we had to go back */
                if (last_valid)
                    offset = last_valid;
                break;
            }
            offset += inc;
            dh->c.length += inc;
            nf9->flows += r;
            last_valid = 0;           /* Don't clobber this header now */
        }
        /* Don't finish header if it has already been done */
        if (dh != NULL) {
            if (offset % 4 != 0) {
                /* Pad to multiple of 4 */
                dh->c.length += 4 - (offset % 4);
                offset += 4 - (offset % 4);
            }
            /* Finalise last header */
            dh->c.length = htons (dh->c.length);
        }
        nf9->flows = htons (nf9->flows);
        nf9->sequence = htonl(first_seq + num_packets);

        // Send!
        send(m_sockets[threadid], packet, (size_t)offset, 0);

        nf9_pkts_until_template--;
        j += i;
    }
}

int Netflow9Generator::generate_flow(u_char * packet, u_int len, const struct timeval *system_boot_time, u_int * len_used) {
    struct NF9_SOFTFLOWD_DATA_V4 d4;
    struct NF9_SOFTFLOWD_DATA_COMMON *dc;
    u_int freclen, ret_len, nflows;

    memset (&d4, 0, sizeof (d4));
    *len_used = nflows = ret_len = 0;
    
    // IPv4 flow
    freclen = sizeof (struct NF9_SOFTFLOWD_DATA_V4);
    d4.src_addr = m_random_dis(m_random_gen);
    d4.dst_addr = m_random_dis(m_random_gen);

    dc = &(d4.c);
    dc->ipproto = 4;
    dc->first_switched = htonl(0);
    dc->last_switched = htonl(1);
    dc->bytes = htonl(42);
    dc->packets = htonl (1);
    dc->if_index_in = dc->if_index_out = htonl (99);
    dc->src_port = 99;
    dc->protocol = 1;

    if (ret_len + freclen > len)
        return (-1);
    memcpy (packet + ret_len, &d4, freclen);
    ret_len += freclen;
    nflows++;

    *len_used = ret_len;
    return (nflows);
}

void Netflow9Generator::init_template() {
    memset (&v4_template, 0, sizeof (v4_template));
    v4_template.h.c.flowset_id = htons (NFLOW9_TEMPLATE_SET_ID);
    v4_template.h.c.length = htons (sizeof (v4_template));
    v4_template.h.template_id = htons (NF9_SOFTFLOWD_V4_TEMPLATE_ID);
    v4_template.h.count = htons (NF9_SOFTFLOWD_TEMPLATE_NRECORDS);
    v4_template.r[0].type = htons (NF9_IPV4_SRC_ADDR);
    v4_template.r[0].length = htons (4);
    v4_template.r[1].type = htons (NF9_IPV4_DST_ADDR);
    v4_template.r[1].length = htons (4);
    v4_template.r[2].type = htons (NF9_LAST_SWITCHED);
    v4_template.r[2].length = htons (4);
    v4_template.r[3].type = htons (NF9_FIRST_SWITCHED);
    v4_template.r[3].length = htons (4);
    v4_template.r[4].type = htons (NF9_IN_BYTES);
    v4_template.r[4].length = htons (4);
    v4_template.r[5].type = htons (NF9_IN_PACKETS);
    v4_template.r[5].length = htons (4);
    v4_template.r[6].type = htons (NF9_IF_INDEX_IN);
    v4_template.r[6].length = htons (4);
    v4_template.r[7].type = htons (NF9_IF_INDEX_OUT);
    v4_template.r[7].length = htons (4);
    v4_template.r[8].type = htons (NF9_L4_SRC_PORT);
    v4_template.r[8].length = htons (2);
    v4_template.r[9].type = htons (NF9_L4_DST_PORT);
    v4_template.r[9].length = htons (2);
    v4_template.r[10].type = htons (NF9_PROTOCOL);
    v4_template.r[10].length = htons (1);
    v4_template.r[11].type = htons (NF9_TCP_FLAGS);
    v4_template.r[11].length = htons (1);
    v4_template.r[12].type = htons (NF9_IP_PROTOCOL_VERSION);
    v4_template.r[12].length = htons (1);
    v4_template.r[13].type = htons (NF9_TOS);
    v4_template.r[13].length = htons (1);
    v4_template.r[14].type = htons (NF9_ICMP_TYPE);
    v4_template.r[14].length = htons (2);
    v4_template.r[15].type = htons (NF9_SRC_VLAN);
    v4_template.r[15].length = htons (2);
    memset (&v6_template, 0, sizeof (v6_template));
    v6_template.h.c.flowset_id = htons (NFLOW9_TEMPLATE_SET_ID);
    v6_template.h.c.length = htons (sizeof (v6_template));
    v6_template.h.template_id = htons (NF9_SOFTFLOWD_V6_TEMPLATE_ID);
    v6_template.h.count = htons (NF9_SOFTFLOWD_TEMPLATE_NRECORDS);
    v6_template.r[0].type = htons (NF9_IPV6_SRC_ADDR);
    v6_template.r[0].length = htons (16);
    v6_template.r[1].type = htons (NF9_IPV6_DST_ADDR);
    v6_template.r[1].length = htons (16);
    v6_template.r[2].type = htons (NF9_LAST_SWITCHED);
    v6_template.r[2].length = htons (4);
    v6_template.r[3].type = htons (NF9_FIRST_SWITCHED);
    v6_template.r[3].length = htons (4);
    v6_template.r[4].type = htons (NF9_IN_BYTES);
    v6_template.r[4].length = htons (4);
    v6_template.r[5].type = htons (NF9_IN_PACKETS);
    v6_template.r[5].length = htons (4);
    v6_template.r[6].type = htons (NF9_IF_INDEX_IN);
    v6_template.r[6].length = htons (4);
    v6_template.r[7].type = htons (NF9_IF_INDEX_OUT);
    v6_template.r[7].length = htons (4);
    v6_template.r[8].type = htons (NF9_L4_SRC_PORT);
    v6_template.r[8].length = htons (2);
    v6_template.r[9].type = htons (NF9_L4_DST_PORT);
    v6_template.r[9].length = htons (2);
    v6_template.r[10].type = htons (NF9_PROTOCOL);
    v6_template.r[10].length = htons (1);
    v6_template.r[11].type = htons (NF9_TCP_FLAGS);
    v6_template.r[11].length = htons (1);
    v6_template.r[12].type = htons (NF9_IP_PROTOCOL_VERSION);
    v6_template.r[12].length = htons (1);
    v6_template.r[13].type = htons (NF9_TOS);
    v6_template.r[13].length = htons (1);
    v6_template.r[14].type = htons (NF9_ICMP_TYPE);
    v6_template.r[14].length = htons (2);
    v6_template.r[15].type = htons (NF9_SRC_VLAN);
    v6_template.r[15].length = htons (2);

    // Initialize the options template
    u_int16_t ifidx = 1;
    uint32_t sampling_interval = 1;

    memset (&option_template, 0, sizeof (option_template));
    option_template.h.c.flowset_id = htons (NFLOW9_OPTION_TEMPLATE_SET_ID);
    option_template.h.c.length = htons (sizeof (option_template));
    option_template.h.template_id = htons (NF9_SOFTFLOWD_OPTION_TEMPLATE_ID);
    option_template.h.scope_length = htons (sizeof (option_template.s));
    option_template.h.option_length = htons (sizeof (option_template.r));
    option_template.s[0].type = htons (NF9_OPTION_SCOPE_INTERFACE);
    option_template.s[0].length = htons (sizeof (option_data.scope_ifidx));
    option_template.r[0].type = htons (NFLOW9_SAMPLING_INTERVAL);
    option_template.r[0].length =
            htons (sizeof (option_data.sampling_interval));
    option_template.r[1].type = htons (NFLOW9_SAMPLING_ALGORITHM);
    option_template.r[1].length =
            htons (sizeof (option_data.sampling_algorithm));

    memset (&option_data, 0, sizeof (option_data));
    option_data.c.flowset_id = htons (NF9_SOFTFLOWD_OPTION_TEMPLATE_ID);
    option_data.c.length = htons (sizeof (option_data));
    option_data.scope_ifidx = htonl (ifidx);
    option_data.sampling_interval = htonl (sampling_interval);
    option_data.sampling_algorithm = NFLOW9_SAMPLING_ALGORITHM_DETERMINISTIC;
}
