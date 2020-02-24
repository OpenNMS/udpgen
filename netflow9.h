/*
 * This code is largely adapted from softflowd
 *
 * Copyright 2002 Damien Miller <djm@mindrot.org> All rights reserved.
 * Copyright 2019 Hitoshi Irino <irino@sfc.wide.ad.jp> All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS    OR
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

#ifndef UDPGEN_NETFLOW9_H
#define UDPGEN_NETFLOW9_H

#if defined(__GNUC__)
#ifndef __dead
#define __dead                __attribute__((__noreturn__))
#endif
#ifndef __packed
#define __packed              __attribute__((__packed__))
#endif
#endif

#define NFLOW9_TEMPLATE_SET_ID          0
#define NFLOW9_OPTION_TEMPLATE_SET_ID   1

/* Information Elements */
#define NFLOW9_SAMPLING_INTERVAL        34
#define NFLOW9_SAMPLING_ALGORITHM       35

#define NFLOW9_OPTION_SCOPE_INTERFACE           2
#define NFLOW9_SAMPLING_ALGORITHM_DETERMINISTIC 1

struct NFLOW9_HEADER {
    u_int16_t version, flows;
    u_int32_t uptime_ms;
    u_int32_t export_time;        // in seconds
    u_int32_t sequence, od_id;
} __packed;

/* Netflow v.9 */
struct NF9_FLOWSET_HEADER_COMMON {
    u_int16_t flowset_id, length;
} __packed;
struct NF9_TEMPLATE_FLOWSET_HEADER {
    struct NF9_FLOWSET_HEADER_COMMON c;
    u_int16_t template_id, count;
} __packed;
struct NF9_OPTION_TEMPLATE_FLOWSET_HEADER {
    struct NF9_FLOWSET_HEADER_COMMON c;
    u_int16_t template_id, scope_length, option_length;
} __packed;
struct NF9_TEMPLATE_FLOWSET_RECORD {
    u_int16_t type, length;
} __packed;
struct NF9_DATA_FLOWSET_HEADER {
    struct NF9_FLOWSET_HEADER_COMMON c;
} __packed;
#define NF9_MIN_RECORD_FLOWSET_ID	256

/* Flowset record types the we care about */
#define NF9_IN_BYTES			1
#define NF9_IN_PACKETS			2
/* ... */
#define NF9_PROTOCOL			4
#define NF9_TOS				5
/* ... */
#define NF9_TCP_FLAGS			6
#define NF9_L4_SRC_PORT			7
#define NF9_IPV4_SRC_ADDR		8
/* ... */
#define NF9_IF_INDEX_IN			10
#define NF9_L4_DST_PORT			11
#define NF9_IPV4_DST_ADDR		12
/* ... */
#define NF9_IF_INDEX_OUT		14
/* ... */
#define NF9_LAST_SWITCHED		21
#define NF9_FIRST_SWITCHED		22
/* ... */
#define NF9_IPV6_SRC_ADDR		27
#define NF9_IPV6_DST_ADDR		28
/* ... */
#define NF9_ICMP_TYPE		        32
/* ... */
#define NF9_SRC_VLAN                    58
/* ... */
#define NF9_IP_PROTOCOL_VERSION		60

/* Stuff pertaining to the templates that softflowd uses */
#define NF9_SOFTFLOWD_TEMPLATE_NRECORDS	16
struct NF9_SOFTFLOWD_TEMPLATE {
    struct NF9_TEMPLATE_FLOWSET_HEADER h;
    struct NF9_TEMPLATE_FLOWSET_RECORD r[NF9_SOFTFLOWD_TEMPLATE_NRECORDS];
} __packed;

#define NF9_SOFTFLOWD_OPTION_TEMPLATE_SCOPE_RECORDS	1
#define NF9_SOFTFLOWD_OPTION_TEMPLATE_NRECORDS	2
struct NF9_SOFTFLOWD_OPTION_TEMPLATE {
    struct NF9_OPTION_TEMPLATE_FLOWSET_HEADER h;
    struct NF9_TEMPLATE_FLOWSET_RECORD
            s[NF9_SOFTFLOWD_OPTION_TEMPLATE_SCOPE_RECORDS];
    struct NF9_TEMPLATE_FLOWSET_RECORD
            r[NF9_SOFTFLOWD_OPTION_TEMPLATE_NRECORDS];
} __packed;

/* softflowd data flowset types */
struct NF9_SOFTFLOWD_DATA_COMMON {
    u_int32_t last_switched, first_switched;
    u_int32_t bytes, packets;
    u_int32_t if_index_in, if_index_out;
    u_int16_t src_port, dst_port;
    u_int8_t protocol, tcp_flags, ipproto, tos;
    u_int16_t icmp_type, vlanid;
} __packed;

struct NF9_SOFTFLOWD_DATA_V4 {
    u_int32_t src_addr, dst_addr;
    struct NF9_SOFTFLOWD_DATA_COMMON c;
} __packed;

struct NF9_SOFTFLOWD_DATA_V6 {
    u_int8_t src_addr[16], dst_addr[16];
    struct NF9_SOFTFLOWD_DATA_COMMON c;
} __packed;

struct NF9_SOFTFLOWD_OPTION_DATA {
    struct NF9_FLOWSET_HEADER_COMMON c;
    u_int32_t scope_ifidx;
    u_int32_t sampling_interval;
    u_int8_t sampling_algorithm;
    u_int8_t padding[3];
} __packed;

/* Local data: templates and counters */
#define NF9_SOFTFLOWD_MAX_PACKET_SIZE	512
#define NF9_SOFTFLOWD_V4_TEMPLATE_ID	1024
#define NF9_SOFTFLOWD_V6_TEMPLATE_ID	2048
#define NF9_SOFTFLOWD_OPTION_TEMPLATE_ID	256

#define NF9_DEFAULT_TEMPLATE_INTERVAL	16

/* ... */
#define NF9_OPTION_SCOPE_SYSTEM    1
#define NF9_OPTION_SCOPE_INTERFACE 2
#define NF9_OPTION_SCOPE_LINECARD  3
#define NF9_OPTION_SCOPE_CACHE     4
#define NF9_OPTION_SCOPE_TEMPLATE  5

static struct NF9_SOFTFLOWD_TEMPLATE v4_template;
static struct NF9_SOFTFLOWD_TEMPLATE v6_template;
static struct NF9_SOFTFLOWD_OPTION_TEMPLATE option_template;
static struct NF9_SOFTFLOWD_OPTION_DATA option_data;
static int nf9_pkts_until_template = -1;


#endif //UDPGEN_NETFLOW9_H
