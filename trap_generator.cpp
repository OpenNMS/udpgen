#include "trap_generator.hpp"
#include <iostream>
#include <cstring>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

oid objid_sysuptime[] = { 1, 3, 6, 1, 2, 1, 1, 3, 0 };
oid trap_oid[] = { 1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0 };
oid objid_id[] = { 1, 3, 6, 1, 6, 3, 1, 1, 5, 1 };

TrapGenerator::TrapGenerator() : m_session(NULL) {
    setPort(1162);
}

TrapGenerator::~TrapGenerator() { }

int TrapGenerator::start() {
    char peername[512];
    snprintf(peername, 512, "%s:%d", getHost(), getPort());

    m_session = new netsnmp_session();
    snmp_sess_init(m_session);
    m_session->version = SNMP_VERSION_2c;
    m_session->community = (u_char*)m_community;
    m_session->community_len = strlen((const char*)m_session->community);
    m_session->peername = (char*)(peername);

    m_sessions = std::unique_ptr<netsnmp_session*[]>(new netsnmp_session*[getNumThreads()]);
    for (int i = 0; i < getNumThreads(); i++) {
        netsnmp_session *ss;
        ss = snmp_open(m_session);
        if (!ss) {
            snmp_sess_perror("ack", m_session);
            return -1;
        }
        m_sessions[i] = ss;
    }

    m_template_pdu = snmp_pdu_create(SNMP_MSG_TRAP2);
    if (m_template_pdu == NULL) {
        std::cout << "PDU creation failed." << std::endl;
        return -1;
    }
    m_template_pdu->community = (u_char*)m_community;
    m_template_pdu->community_len = strlen(m_community);
    m_template_pdu->trap_type = SNMP_TRAP_ENTERPRISESPECIFIC;

    long sysuptime;
    char csysuptime[20];
    sysuptime = get_uptime ();
    sprintf (csysuptime, "%ld", sysuptime);
    char *trap = csysuptime;
    snmp_add_var (m_template_pdu, objid_sysuptime, sizeof (objid_sysuptime)/sizeof(oid),'t', trap);
    snmp_add_var(m_template_pdu, trap_oid, OID_LENGTH(trap_oid), 'o', ".1.3.6.1.1.6.3.1.1.5.1");
    snmp_add_var(m_template_pdu, objid_id, OID_LENGTH(objid_id) , 's', "ABC");

    return UDPGenerator::start();
}

void TrapGenerator::stop() {
    UDPGenerator::stop();
    for (int i = 0; i < getNumThreads(); i++) {
        netsnmp_session *ss = m_sessions[i];
        if (ss != NULL) {
            snmp_close(ss);
            m_sessions[i] = NULL;
        }
    }

    if (m_session != NULL) {
        // Causes SEGFAULT
        //snmp_sess_close(session);
        delete m_session;
        m_session = NULL;
    }

    if (m_template_pdu != NULL) {
        // Causes SEGFAULT
        //snmp_free_pdu(template_pdu);
        m_template_pdu = NULL;
    }
}

const char* TrapGenerator::getPacketDescription() {
    return "SNMP Traps";
}

void TrapGenerator::sendPackets(int threadid, unsigned int num_packets, unsigned long long first_seq) {
    netsnmp_session *ss = m_sessions[threadid];
    for (unsigned int i = 0; i < num_packets; i++) {
        send_trap_to_sess(ss, m_template_pdu);
    }
}
