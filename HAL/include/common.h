#ifndef __COMMON_H__
#define __COMMON_H__

#include <arpa/inet.h>
#include <array>
#include <assert.h>
#include <stdint.h>

// definition of ether_addr and ether_header
#include <net/ethernet.h>

// definition of in6_addr
#include <netinet/in.h>

// definition of ip6_hdr
#include <netinet/ip6.h>

// definition of udphdr
#include <netinet/udp.h>

// definition of icmp6_hdr
#include <netinet/icmp6.h>

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
void get_ipv6_mcast_mac(const in6_addr mcast_ip, ether_addr *mac);

// non thread-safe, beware!
// don't call it twice in a same call to printf!
const char *inet6_ntoa(const in6_addr addr);
const char *ether_ntoa(const ether_addr mac);

#endif