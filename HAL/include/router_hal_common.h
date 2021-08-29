#ifndef __ROUTER_HAL_COMMON_H__
#define __ROUTER_HAL_COMMON_H__

// you should not include this file in your own code.
#include "router_hal.h"
#include <map>
#include <time.h>

bool inited = false;
int debugEnabled = 0;
in6_addr interface_addrs[N_IFACE_ON_BOARD] = {0};
ether_addr interface_mac[N_IFACE_ON_BOARD] = {0};
in6_addr interface_link_local_addrs[N_IFACE_ON_BOARD] = {0};

std::map<std::pair<in6_addr, int>, ether_addr> ndp_table;
std::map<std::pair<in6_addr, int>, uint64_t> ndp_timer;

extern "C" {

uint64_t HAL_GetTicks() {
  struct timespec tp = {0};
  clock_gettime(CLOCK_MONOTONIC, &tp);
  // millisecond
  return (uint64_t)tp.tv_sec * 1000 + (uint64_t)tp.tv_nsec / 1000000;
}
}

#endif
