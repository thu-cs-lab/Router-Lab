#ifndef __ROUTER_HAL_COMMON_H__
#define __ROUTER_HAL_COMMON_H__

// don't include this file in your own code.
#include "router_hal.h"
#include <string.h>

// send igmp join to the multicast address
void HAL_JoinIGMPGroup(int if_index, in_addr_t ip) {
  uint8_t buffer[40] = {
      0x46, 0xc0, 0x00, 0x28, 0x00, 0x00, 0x40, 0x00, 0x01, 0x02, // Header
      0x00, 0x00,             // Header checksum
      0x00, 0x00, 0x00, 0x00, // Source
      0xe0, 0x00, 0x00, 0x16, // IGMP Multicast
      0x94, 0x04, 0x00, 0x00, // IP Option
      0x22,                   // Membership Report
      0x00,                   // Zeros
      0xf9, 0xf4,             // Checksum
      0x00, 0x00,             // Zeros
      0x00, 0x01,             // One Group Record
      0x04,                   // Record Type
      0x00, 0x00, 0x00,       // Zeros
      0xe0, 0x00, 0x00, 0x09  // RIP Multicast
  };
  memcpy(&buffer[12], &ip, sizeof(in_addr_t));
  uint32_t ip_chksum = 0;
  for (int i = 0; i < 12; i++) {
    ip_chksum += ((uint16_t *)buffer)[i];
  }
  while (ip_chksum >= 0x10000) {
    ip_chksum -= 0x10000;
    ip_chksum += 1;
  }
  uint16_t chksum = ~ip_chksum;
  memcpy(&buffer[10], &chksum, sizeof(uint16_t));
  macaddr_t dst_mac = {0x01, 0x00, 0x5e, 0x00, 0x00, 0x16};
  HAL_SendIPPacket(if_index, buffer, sizeof(buffer), dst_mac);
}

#endif
