#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>
#include <array>

// MAC address and IPv6 address
typedef std::array<uint8_t, 6> macaddr_t;
typedef std::array<uint8_t, 16> ipv6addr_t;

#endif