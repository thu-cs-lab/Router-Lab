#include "router_hal.h"
#include "router_hal_common.h"
#include <stdio.h>

#include <emscripten.h>
#include <map>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const int IP_OFFSET = 14;

bool inited = false;
int debugEnabled = 0;
in_addr_t interface_addrs[N_IFACE_ON_BOARD] = {0};
macaddr_t interface_mac[N_IFACE_ON_BOARD] = {0};

std::map<std::pair<in_addr_t, int>, macaddr_t> arp_table;
std::map<std::pair<in_addr_t, int>, uint64_t> arp_timer;

extern "C" {
int HAL_Init(HAL_IN int debug, HAL_IN in_addr_t if_addrs[N_IFACE_ON_BOARD]) {
  if (inited) {
    return 0;
  }
  debugEnabled = debug;

  inited = true;
  return 0;
}

uint64_t HAL_GetTicks() {
  struct timespec tp = {0};
  clock_gettime(CLOCK_MONOTONIC, &tp);
  // millisecond
  return (uint64_t)tp.tv_sec * 1000 + (uint64_t)tp.tv_nsec / 1000000;
}

int HAL_ArpGetMacAddress(HAL_IN int if_index, HAL_IN in_addr_t ip,
                         HAL_OUT macaddr_t o_mac) {
  if (!inited) {
    return HAL_ERR_CALLED_BEFORE_INIT;
  }
  if (if_index >= N_IFACE_ON_BOARD || if_index < 0) {
    return HAL_ERR_INVALID_PARAMETER;
  }
  return HAL_ERR_IP_NOT_EXIST;
}

int HAL_GetInterfaceMacAddress(HAL_IN int if_index, HAL_OUT macaddr_t o_mac) {
  if (!inited) {
    return HAL_ERR_CALLED_BEFORE_INIT;
  }
  if (if_index >= N_IFACE_ON_BOARD || if_index < 0) {
    return HAL_ERR_IFACE_NOT_EXIST;
  }

  memcpy(o_mac, interface_mac[if_index], sizeof(macaddr_t));
  return 0;
}

extern size_t read_packet(HAL_IN int if_index_mask, HAL_OUT uint8_t *buffer,
                          HAL_IN size_t length, HAL_OUT macaddr_t src_mac,
                          HAL_OUT macaddr_t dst_mac, HAL_IN int64_t timeout,
                          HAL_OUT int *if_index);

int HAL_ReceiveIPPacket(HAL_IN int if_index_mask, HAL_OUT uint8_t *buffer,
                        HAL_IN size_t length, HAL_OUT macaddr_t src_mac,
                        HAL_OUT macaddr_t dst_mac, HAL_IN int64_t timeout,
                        HAL_OUT int *if_index) {
  if (!inited) {
    return HAL_ERR_CALLED_BEFORE_INIT;
  }
  if ((if_index_mask & ((1 << N_IFACE_ON_BOARD) - 1)) == 0 ||
      (timeout < 0 && timeout != -1) || (if_index == NULL) ||
      (buffer == NULL)) {
    return HAL_ERR_INVALID_PARAMETER;
  }

  return read_packet(if_index_mask, buffer, length, src_mac, dst_mac, timeout,
                     if_index);
}

extern void send_packet(HAL_IN int if_index, HAL_IN uint8_t *buffer,
                        HAL_IN size_t length, HAL_IN macaddr_t dst_mac);
int HAL_SendIPPacket(HAL_IN int if_index, HAL_IN uint8_t *buffer,
                     HAL_IN size_t length, HAL_IN macaddr_t dst_mac) {
  if (!inited) {
    return HAL_ERR_CALLED_BEFORE_INIT;
  }
  if (if_index >= N_IFACE_ON_BOARD || if_index < 0) {
    return HAL_ERR_INVALID_PARAMETER;
  }
  send_packet(if_index, buffer, length, dst_mac);
  return 0;
}
}
