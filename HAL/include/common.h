#ifndef __COMMON_H__
#define __COMMON_H__

#include <arpa/inet.h>
#include <array>
#include <assert.h>
#include <stdint.h>

// definition of in6_addr
#include <netinet/in.h>

// definition of ip6_hdr
#include <netinet/ip6.h>

// definition of udphdr
#include <netinet/udp.h>

// definition of icmp6_hdr
#include <netinet/icmp6.h>

// MAC address type
typedef uint8_t macaddr_t[6];

typedef struct ether_hdr {
  macaddr_t dst_mac;
  macaddr_t src_mac;
  uint16_t ether_type;
} ether_hdr;

// utility functions
in6_addr operator&(const in6_addr &a, const in6_addr &b);
bool operator!=(const in6_addr &a, const in6_addr &b);
bool operator==(const in6_addr &a, const in6_addr &b);
bool operator<(const in6_addr &a, const in6_addr &b);

// compute solicited-node multicast address
// ref: rfc4291
in6_addr get_solicited_node_mcast_addr(const in6_addr ip);
// compute mac for ipv6 mcast addr
// ref: rfc2464
void get_ipv6_mcast_mac(const in6_addr mcast_ip, macaddr_t mac);

// non thread-safe, beware!
const char *inet6_ntoa(in6_addr addr);

#endif