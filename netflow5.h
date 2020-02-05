/*
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

#include <stdint.h>

/*
 * This is the Cisco Netflow(tm) version 5 packet format
 * Based on:
 * http://www.cisco.com/en/US/products/sw/netmgtsw/ps1964/products_implementation_design_guide09186a00800d6a11.html 
 */
struct NF5_HEADER {
    uint16_t version, flows;
    uint32_t uptime_ms, time_sec, time_nanosec, flow_sequence;
    uint8_t engine_type, engine_id;
    uint16_t sampling_interval;
};
struct NF5_FLOW {
    uint32_t src_ip, dest_ip, nexthop_ip;
    uint16_t if_index_in, if_index_out;
    uint32_t flow_packets, flow_octets;
    uint32_t flow_start, flow_finish;
    uint16_t src_port, dest_port;
    uint8_t pad1;
    uint8_t tcp_flags, protocol, tos;
    uint16_t src_as, dest_as;
    uint8_t src_mask, dst_mask;
    uint16_t pad2;
};
#define NF5_MAXFLOWS		30
#define NF5_MAXPACKET_SIZE	(sizeof(struct NF5_HEADER) + \
				 (NF5_MAXFLOWS * sizeof(struct NF5_FLOW)))
