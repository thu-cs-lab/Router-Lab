#include "router_hal.h"
#include <stdio.h>

#include <map>
#include <pcap.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <utility>

bool inited = false;
bool outputInited = false;
int debugEnabled = 0;
uint32_t interface_addrs[N_IFACE_ON_BOARD] = {0};
ether_addr interface_mac[N_IFACE_ON_BOARD] = {0};

// input
pcap_t *pcap_handle;

// output
pcap_t *pcap_out_handle;
pcap_dumper_t *pcap_dumper;

std::map<std::pair<in6_addr, int>, ether_addr> ndp_table;

extern "C" {
int HAL_Init(HAL_IN int debug, HAL_IN in6_addr if_addrs[N_IFACE_ON_BOARD]) {
  if (inited) {
    return 0;
  }
  debugEnabled = debug;

  for (int i = 0; i < N_IFACE_ON_BOARD; i++) {
    // hard coded MAC
    interface_mac[i] = {2, 3, 3, 0, 0, (uint8_t)i};
    ndp_table[std::pair<in6_addr, int>(if_addrs[i], i)] = interface_mac[i];
  }

  char error_buffer[PCAP_ERRBUF_SIZE];

  // input
  pcap_handle = pcap_open_offline("-", error_buffer);
  if (!pcap_handle) {
    if (debugEnabled) {
      fprintf(stderr, "pcap_open_offline failed with %s", error_buffer);
    }
    return HAL_ERR_UNKNOWN;
  }

  memcpy(interface_addrs, if_addrs, sizeof(interface_addrs));

  inited = true;
  return 0;
}

uint64_t HAL_GetTicks() {
  struct timespec tp = {0};
  clock_gettime(CLOCK_MONOTONIC, &tp);
  return (uint64_t)tp.tv_sec * 1000 + (uint64_t)tp.tv_nsec / 1000000;
}

int HAL_GetNeighborMacAddress(int if_index, in6_addr ip, ether_addr *o_mac) {
  return HAL_ERR_NOT_SUPPORTED;
}

int HAL_GetInterfaceMacAddress(int if_index, ether_addr *o_mac) {
  return HAL_ERR_NOT_SUPPORTED;
}

int HAL_ReceiveIPPacket(int if_index_mask, uint8_t *buffer, size_t length,
                        ether_addr *src_mac, ether_addr *dst_mac,
                        int64_t timeout, int *if_index) {
  if (!inited) {
    return HAL_ERR_CALLED_BEFORE_INIT;
  }
  if ((if_index_mask & ((1 << N_IFACE_ON_BOARD) - 1)) == 0 ||
      (timeout < 0 && timeout != -1) || (if_index == NULL)) {
    return HAL_ERR_INVALID_PARAMETER;
  }

  int64_t begin = HAL_GetTicks();
  int64_t current_time = 0;

  struct pcap_pkthdr *hdr;
  const u_char *packet;
  do {
    int res = pcap_next_ex(pcap_handle, &hdr, &packet);
    if (res == PCAP_ERROR_BREAK) {
      return HAL_ERR_EOF;
    } else if (res != 1) {
      // retry
      continue;
    }

    // check 802.1Q
    uint32_t vlan = (((uint32_t)packet[14]) << 8) + packet[15];
    vlan &= 0xFFF;
    if (packet && hdr->caplen >= sizeof(ether_header) && packet[12] == 0x81 &&
        packet[13] == 0x00 && 0 < vlan && vlan < N_IFACE_ON_BOARD) {
      int current_port = packet[15];
      if (hdr->caplen >= sizeof(ether_header) + 4 && packet[16] == 0x86 &&
          packet[17] == 0xdd) {
        // IPv6
        // assuming len == caplen
        size_t ip_len = hdr->caplen - sizeof(ether_header) - 4;
        size_t real_length = length > ip_len ? ip_len : length;
        memcpy(buffer, &packet[sizeof(ether_header) + 4], real_length);
        memcpy(dst_mac, &packet[0], sizeof(ether_addr));
        memcpy(src_mac, &packet[6], sizeof(ether_addr));
        *if_index = current_port;
        return ip_len;
      }
    }

    // -1 for infinity
  } while ((current_time = HAL_GetTicks()) < begin + timeout || timeout == -1);
  return 0;
}

int HAL_SendIPPacket(HAL_IN int if_index, HAL_IN uint8_t *buffer,
                     HAL_IN size_t length, HAL_IN ether_addr dst_mac) {
  return HAL_ERR_NOT_SUPPORTED;
}
}
