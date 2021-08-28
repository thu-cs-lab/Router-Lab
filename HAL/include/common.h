#ifndef __COMMON_H__
#define __COMMON_H__

#include <array>
#include <stdint.h>
#include <assert.h>
#include <arpa/inet.h>

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

// utility functions
in6_addr operator &(const in6_addr &a, const in6_addr &b);
bool operator !=(const in6_addr &a, const in6_addr &b);
bool operator ==(const in6_addr &a, const in6_addr &b);
bool operator <(const in6_addr &a, const in6_addr &b);

// non thread-safe, beware!
const char *inet6_ntoa(in6_addr addr);

#endif