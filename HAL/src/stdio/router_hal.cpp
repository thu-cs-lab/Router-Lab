#include "router_hal.h"
#include <stdio.h>

#include <map>
#include <pcap.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <utility>

const int IP_OFFSET = 18; // 6 + 6 + 4 + 2

bool inited = false;
bool outputInited = false;
int debugEnabled = 0;
in_addr_t interface_addrs[N_IFACE_ON_BOARD] = {0};
macaddr_t interface_mac[N_IFACE_ON_BOARD] = {0};

// input
pcap_t *pcap_handle;

// output
pcap_t *pcap_out_handle;
pcap_dumper_t *pcap_dumper;

// workaround for clang
struct macaddr_wrap {
  macaddr_t mac;
};

std::map<std::pair<in_addr_t, int>, macaddr_wrap> arp_table;

extern "C" {
int HAL_Init(int debug, in_addr_t if_addrs[N_IFACE_ON_BOARD]) {
  if (inited) {
    return 0;
  }
  debugEnabled = debug;

  for (int i = 0; i < N_IFACE_ON_BOARD; i++) {
    // hard coded MAC
    macaddr_t mac = {2, 3, 3, 0, 0, (uint8_t)i};
    memcpy(interface_mac[i], mac, sizeof(macaddr_t));
    memcpy(&arp_table[std::pair<in_addr_t, int>(if_addrs[i], i)],
           interface_mac[i], sizeof(macaddr_t));
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

int HAL_ArpGetMacAddress(int if_index, in_addr_t ip, macaddr_t o_mac) {
  if (!inited) {
    return HAL_ERR_CALLED_BEFORE_INIT;
  }
  if (if_index >= N_IFACE_ON_BOARD || if_index < 0) {
    return HAL_ERR_INVALID_PARAMETER;
  }

  if ((ip & 0xe0) == 0xe0) {
    uint8_t multicasting_mac[6] = {0x01, 0, 0x5e, (uint8_t)((ip >> 8) & 0x7f), (uint8_t)(ip >> 16), (uint8_t)(ip >> 24)};
    memcpy(o_mac, multicasting_mac, sizeof(macaddr_t));
    return 0;
  }

  auto it = arp_table.find(std::pair<in_addr_t, int>(ip, if_index));
  if (it != arp_table.end()) {
    memcpy(o_mac, &it->second, sizeof(macaddr_t));
    return 0;
  } else {
    if (debugEnabled) {
      struct in_addr addr;
      addr.s_addr = ip;
      fprintf(
          stderr,
          "HAL_ArpGetMacAddress: asking for ip address %s with arp request\n",
          inet_ntoa(addr));
    }
    uint8_t buffer[64] = {0};
    // dst mac = broadcast
    for (int i = 0; i < 6; i++) {
      buffer[i] = 0xff;
    }
    // src mac
    macaddr_t mac;
    HAL_GetInterfaceMacAddress(if_index, mac);
    memcpy(&buffer[6], mac, sizeof(macaddr_t));
    // 802.1Q
    buffer[12] = 0x81;
    buffer[13] = 0x00;
    buffer[14] = 0x00;
    buffer[15] = if_index;
    // ARP
    buffer[16] = 0x08;
    buffer[17] = 0x06;
    // hardware type
    buffer[19] = 0x01;
    // protocol type
    buffer[20] = 0x08;
    // hardware size
    buffer[22] = 0x06;
    // protocol size
    buffer[23] = 0x04;
    // opcode
    buffer[25] = 0x01;
    // sender
    memcpy(&buffer[26], mac, sizeof(macaddr_t));
    memcpy(&buffer[32], &interface_addrs[if_index], sizeof(in_addr_t));
    // target
    memcpy(&buffer[42], &ip, sizeof(in_addr_t));

    struct pcap_pkthdr header;
    header.caplen = header.len = sizeof(buffer);

    struct timespec tp = {0};
    clock_gettime(CLOCK_MONOTONIC, &tp);
    header.ts.tv_sec = tp.tv_sec;
    header.ts.tv_usec = tp.tv_nsec / 1000;

    if (!outputInited) {
      // output
      pcap_out_handle = pcap_open_dead(DLT_EN10MB, 0x40000);
      pcap_dumper = pcap_dump_open(pcap_out_handle, "-");
      outputInited = true;
    }
    pcap_dump((u_char *)pcap_dumper, &header, buffer);
  }
  return HAL_ERR_IP_NOT_EXIST;
}

int HAL_GetInterfaceMacAddress(int if_index, macaddr_t o_mac) {
  if (!inited) {
    return HAL_ERR_CALLED_BEFORE_INIT;
  }
  if (if_index >= N_IFACE_ON_BOARD || if_index < 0) {
    return HAL_ERR_IFACE_NOT_EXIST;
  }

  memcpy(o_mac, interface_mac[if_index], sizeof(macaddr_t));
  return 0;
}

int HAL_ReceiveIPPacket(int if_index_mask, uint8_t *buffer, size_t length,
                        macaddr_t src_mac, macaddr_t dst_mac, int64_t timeout,
                        int *if_index) {
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
    if (packet && hdr->caplen >= IP_OFFSET && packet[12] == 0x81 &&
        packet[13] == 0x00 && packet[14] == 0x00 && packet[15] >= 0 &&
        packet[15] < N_IFACE_ON_BOARD) {
      int current_port = packet[15];
      if (packet[16] == 0x08 && packet[17] == 0x00) {
        // IPv4
        // assuming len == caplen
        size_t ip_len = hdr->caplen - IP_OFFSET;
        size_t real_length = length > ip_len ? ip_len : length;
        memcpy(buffer, &packet[IP_OFFSET], real_length);
        memcpy(dst_mac, &packet[0], sizeof(macaddr_t));
        memcpy(src_mac, &packet[6], sizeof(macaddr_t));
        *if_index = current_port;
        return ip_len;
      } else if (packet[16] == 0x08 && packet[17] == 0x06) {
        // ARP
        macaddr_t mac;
        memcpy(mac, &packet[26], sizeof(macaddr_t));
        in_addr_t ip;
        memcpy(&ip, &packet[32], sizeof(in_addr_t));

        memcpy(&arp_table[std::pair<in_addr_t, int>(ip, current_port)], mac,
               sizeof(macaddr_t));
        if (debugEnabled) {
          struct in_addr addr;
          addr.s_addr = ip;
          fprintf(stderr, "HAL_ReceiveIPPacket: learned MAC address of %s\n",
                  inet_ntoa(addr));
        }

        in_addr_t dst_ip;
        memcpy(&dst_ip, &packet[42], sizeof(in_addr_t));
        if (dst_ip == interface_addrs[current_port] && packet[25] == 0x01) {
          // reply
          uint8_t buffer[64] = {0};
          // dst mac
          memcpy(buffer, &packet[6], sizeof(macaddr_t));
          // src mac
          macaddr_t mac;
          HAL_GetInterfaceMacAddress(current_port, mac);
          memcpy(&buffer[6], mac, sizeof(macaddr_t));
          // VLAN
          buffer[12] = 0x81;
          buffer[13] = 0x00;
          buffer[14] = 0x00;
          buffer[15] = current_port;
          // ARP
          buffer[16] = 0x08;
          buffer[17] = 0x06;
          // hardware type
          buffer[19] = 0x01;
          // protocol type
          buffer[20] = 0x08;
          // hardware size
          buffer[22] = 0x06;
          // protocol size
          buffer[23] = 0x04;
          // opcode
          buffer[25] = 0x02;
          // sender
          memcpy(&buffer[26], mac, sizeof(macaddr_t));
          memcpy(&buffer[32], &dst_ip, sizeof(in_addr_t));
          // target
          memcpy(&buffer[36], &packet[22], sizeof(macaddr_t));
          memcpy(&buffer[42], &packet[28], sizeof(in_addr_t));

          struct pcap_pkthdr header;
          header.caplen = header.len = sizeof(buffer);

          struct timespec tp = {0};
          clock_gettime(CLOCK_MONOTONIC, &tp);
          header.ts.tv_sec = tp.tv_sec;
          header.ts.tv_usec = tp.tv_nsec / 1000;

          if (!outputInited) {
            // output
            pcap_out_handle = pcap_open_dead(DLT_EN10MB, 0x40000);
            pcap_dumper = pcap_dump_open(pcap_out_handle, "-");
            outputInited = true;
          }
          pcap_dump((u_char *)pcap_dumper, &header, buffer);

          if (debugEnabled) {
            struct in_addr addr;
            addr.s_addr = ip;
            fprintf(stderr, "HAL_ReceiveIPPacket: replied ARP to %s\n",
                    inet_ntoa(addr));
          }
        }
        continue;
      }
    }

    // -1 for infinity
  } while ((current_time = HAL_GetTicks()) < begin + timeout || timeout == -1);
  return 0;
}

int HAL_SendIPPacket(int if_index, uint8_t *buffer, size_t length,
                     macaddr_t dst_mac) {
  if (!inited) {
    return HAL_ERR_CALLED_BEFORE_INIT;
  }
  if (if_index >= N_IFACE_ON_BOARD || if_index < 0) {
    return HAL_ERR_INVALID_PARAMETER;
  }
  uint8_t *eth_buffer = (uint8_t *)malloc(length + IP_OFFSET);
  memcpy(eth_buffer, dst_mac, sizeof(macaddr_t));
  memcpy(&eth_buffer[6], interface_mac[if_index], sizeof(macaddr_t));
  // VLAN
  eth_buffer[12] = 0x81;
  eth_buffer[13] = 0x00;
  eth_buffer[14] = 0x00;
  eth_buffer[15] = if_index;
  // IPv4
  eth_buffer[16] = 0x08;
  eth_buffer[17] = 0x00;
  memcpy(&eth_buffer[IP_OFFSET], buffer, length);
  struct pcap_pkthdr header;
  header.caplen = header.len = length + IP_OFFSET;

  struct timespec tp = {0};
  clock_gettime(CLOCK_MONOTONIC, &tp);
  header.ts.tv_sec = tp.tv_sec;
  header.ts.tv_usec = tp.tv_nsec / 1000;

  if (!outputInited) {
    // output
    pcap_out_handle = pcap_open_dead(DLT_EN10MB, 0x40000);
    pcap_dumper = pcap_dump_open(pcap_out_handle, "-");
    outputInited = true;
  }
  pcap_dump((u_char *)pcap_dumper, &header, eth_buffer);
  free(eth_buffer);
  return 0;
}
}