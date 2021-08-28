#ifndef __COMMON_H__
#define __COMMON_H__

#include <array>
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

#endif