#ifndef _net_service_
#define _net_service_

#define TCP 6
#define UDP 17
#define ICMP 1
#define ESP 50
#define AH 51

// append SERVICE to prevent name collision
#define SERVICE_FTP_DATA 20
#define SERVICE_FTP 21
#define SERVICE_SSH 22
#define SERVICE_TELNET 23
#define SERVICE_SMTP 25
#define SERVICE_DNS 53
#define SERVICE_BOOTPS 67
#define SERVICE_BOOTPC 68
#define SERVICE_HTTP 80
#define SERVICE_POP3 110
#define SERVICE_NTP 123
#define SERVICE_IMAP 143
#define SERVICE_SNMP 161
#define SERVICE_SNMPTRAP 162
#define SERVICE_BGP 179
#define SERVICE_AT_RTMP 201
#define SERVICE_LDAP 389
#define SERVICE_HTTPS 443
#define SERVICE_ISAKMP 500
#define SERVICE_SYSLOG 514
#define SERVICE_SUBMISSION 587
#define SERVICE_IMAPS 993
#define SERVICE_L2F 1701
#define SERVICE_MS_WBT_SERVER 3389
#define SERVICE_MYSQL 3306
#define SERVICE_IPSEC_NAT_T 4500

struct Service {
  int port;
  int protocol;
};

// a list of common network service
const Service services[] = {
  {SERVICE_FTP_DATA, TCP},
  {SERVICE_FTP, TCP},
  {SERVICE_SSH, TCP},
  {SERVICE_TELNET, TCP},
  {SERVICE_SMTP, TCP},
  {SERVICE_DNS, UDP},
  {SERVICE_BOOTPC, UDP},
  {SERVICE_BOOTPS, UDP},
  {SERVICE_HTTP, TCP},
  {SERVICE_POP3, TCP},
  {SERVICE_NTP, UDP},
  {SERVICE_IMAP, TCP},
  {SERVICE_SNMP, UDP},
  {SERVICE_SNMPTRAP, UDP},
  {SERVICE_BGP, TCP},
  {SERVICE_AT_RTMP, TCP},
  {SERVICE_LDAP, TCP},
  {SERVICE_HTTPS, TCP},
  {SERVICE_ISAKMP, UDP},
  {SERVICE_SYSLOG, UDP},
  {SERVICE_SUBMISSION, TCP},
  {SERVICE_IMAPS, TCP},
  {SERVICE_L2F, UDP},
  {SERVICE_MS_WBT_SERVER, TCP},
  {SERVICE_MYSQL, TCP},
  {SERVICE_IPSEC_NAT_T, UDP},
  {0, ICMP},
  {0, ESP},
  {0, AH}
};

const int services_size = sizeof(services)/sizeof(Service);

#endif
