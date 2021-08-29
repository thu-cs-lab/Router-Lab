#ifndef __COMMON_H__
#define __COMMON_H__

#include <arpa/inet.h>
#include <array>
#include <assert.h>
#include <stdint.h>
#include <type_traits>

// definition of ether_addr and ether_header
#include <net/ethernet.h>
static_assert(sizeof(ether_addr) == 6, "sizeof(ether_addr) should be 6");
static_assert(sizeof(ether_header) == 14, "sizeof(ether_header) should be 14");

// definition of in6_addr
#include <netinet/in.h>
static_assert(sizeof(in6_addr) == 16, "sizeof(in6_addr) should be 16");

// definition of ip6_hdr
#include <netinet/ip6.h>
static_assert(sizeof(ip6_hdr) == 40, "sizeof(ip6_hdr) should be 40");

// definition of udphdr
#include <netinet/udp.h>
static_assert(sizeof(udphdr) == 8, "sizeof(udphdr) should be 8");

// definition of icmp6_hdr
#include <netinet/icmp6.h>
static_assert(sizeof(icmp6_hdr) == 8, "sizeof(icmp6_hdr) should be 8");

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
in6_addr inet6_pton(const char *addr);

#endif