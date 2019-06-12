#include "router_hal.h"
#include <stdio.h>

#include <ifaddrs.h>
#include <linux/if_packet.h>
#include <map>
#include <net/if.h>
#include <net/if_arp.h>
#include <pcap.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <utility>

const int IP_OFFSET = 14;

bool inited = false;
int debugEnabled = 0;
const char *interfaces[N_IFACE_ON_BOARD] = {
    "eth0",
    "eth1",
    "eth2",
    "eth3",
};
in_addr_t interface_addrs[N_IFACE_ON_BOARD] = {0};

pcap_t *pcap_in_handles[N_IFACE_ON_BOARD];
pcap_t *pcap_out_handles[N_IFACE_ON_BOARD];

std::map<std::pair<in_addr_t, int>, macaddr_t> arp_table;

extern "C" {
int HAL_Init(int debug, in_addr_t if_addrs[N_IFACE_ON_BOARD]) {
  if (inited) {
    return 0;
  }
  debugEnabled = debug;

  char error_buffer[PCAP_ERRBUF_SIZE];
  for (int i = 0; i < N_IFACE_ON_BOARD; i++) {
    pcap_in_handles[i] =
        pcap_open_live(interfaces[i], BUFSIZ, 1, 0, error_buffer);
    if (pcap_in_handles[i]) {
      pcap_activate(pcap_in_handles[i]);
      if (debugEnabled) {
        fprintf(stderr, "HAL_Init: pcap capture enabled for %s\n",
                interfaces[i]);
      }
    }
    pcap_out_handles[i] =
        pcap_open_live(interfaces[i], BUFSIZ, 1, 0, error_buffer);
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

  auto it = arp_table.find(std::pair<in_addr_t, int>(ip, if_index));
  if (it != arp_table.end()) {
    memcpy(o_mac, it->second, sizeof(macaddr_t));
    return 0;
  } else {
    if (debugEnabled) {
      fprintf(
          stderr,
          "HAL_ArpGetMacAddress: asking for ip address %s with arp request\n",
          inet_ntoa(in_addr{ip}));
    }
    uint8_t buffer[64] = {0};
    // dst mac
    for (int i = 0; i < 6; i++) {
      buffer[i] = 0xff;
    }
    // src mac
    macaddr_t mac;
    HAL_GetInterfaceMacAddress(if_index, mac);
    memcpy(&buffer[6], mac, sizeof(macaddr_t));
    // ARP
    buffer[12] = 0x08;
    buffer[13] = 0x06;
    // hardware type
    buffer[15] = 0x01;
    // protocol type
    buffer[16] = 0x08;
    // hardware size
    buffer[18] = 0x06;
    // protocol size
    buffer[19] = 0x04;
    // protocol
    buffer[21] = 0x01;
    // sender
    memcpy(&buffer[22], mac, sizeof(macaddr_t));
    memcpy(&buffer[28], &interface_addrs[if_index], sizeof(in_addr_t));
    // target
    memcpy(&buffer[38], &ip, sizeof(in_addr_t));

    pcap_inject(pcap_out_handles[if_index], buffer, 64);
    return HAL_ERR_IP_NOT_EXIST;
  }
}

int HAL_GetInterfaceMacAddress(int if_index, macaddr_t o_mac) {
  if (!inited) {
    return HAL_ERR_CALLED_BEFORE_INIT;
  }

  struct ifaddrs *ifaddr, *ifa;
  if (getifaddrs(&ifaddr) < 0) {
    if (debugEnabled) {
      fprintf(stderr, "HAL_GetInterfaceMacAddress: getifaddrs failed with %s\n",
              strerror(errno));
    }
    return HAL_ERR_UNKNOWN;
  }

  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL)
      continue;
    if (ifa->ifa_addr->sa_family == AF_PACKET &&
        strcmp(ifa->ifa_name, interfaces[if_index]) == 0) {
      // found
      memcpy(o_mac, ((struct sockaddr_ll *)ifa->ifa_addr)->sll_addr,
             sizeof(macaddr_t));
      return 0;
    }
  }
  return HAL_ERR_IFACE_NOT_EXIST;
}

int HAL_ReceiveIPPacket(int if_index_mask, uint8_t *buffer, size_t length,
                        macaddr_t src_mac, macaddr_t dst_mac, int64_t timeout,
                        int *if_index) {
  if (!inited) {
    return HAL_ERR_CALLED_BEFORE_INIT;
  }
  if ((if_index_mask & ((1 << N_IFACE_ON_BOARD) - 1)) == 0 || timeout < 0) {
    return HAL_ERR_INVALID_PARAMETER;
  }

  bool flag = false;
  for (int i = 0; i < N_IFACE_ON_BOARD; i++) {
    if (pcap_in_handles[i] && (if_index_mask & (1 << i))) {
      flag = true;
    }
  }
  if (!flag) {
    if (debugEnabled) {
      fprintf(stderr,
              "HAL_ReceiveIPPacket: no viable interfaces open for capture\n");
    }
    return HAL_ERR_IFACE_NOT_EXIST;
  }

  int64_t begin = HAL_GetTicks();
  int64_t current_time = 0;
  // Round robin
  int current_port = 0;
  struct pcap_pkthdr hdr;
  while ((current_time = HAL_GetTicks()) < begin + timeout) {
    if (if_index_mask & (1 << current_port) == 0 ||
        !pcap_in_handles[current_port]) {
      current_port = (current_port + 1) % N_IFACE_ON_BOARD;
      continue;
    }

    int current_timeout = begin + timeout - current_time;
    // poll, but with low latency
    if (current_timeout > 5) {
      current_timeout = 5;
    }
    pcap_set_timeout(pcap_in_handles[current_port],
                     current_timeout / N_IFACE_ON_BOARD);
    const uint8_t *packet = pcap_next(pcap_in_handles[current_port], &hdr);
    if (packet && hdr.caplen >= IP_OFFSET && packet[12] == 0x08 &&
        packet[13] == 0x00) {
      // IPv4
      // TODO: what if len != caplen
      size_t ip_len = hdr.caplen - IP_OFFSET;
      size_t real_length = length > ip_len ? ip_len : length;
      memcpy(buffer, &packet[IP_OFFSET], real_length);
      memcpy(dst_mac, &packet[0], sizeof(macaddr_t));
      memcpy(src_mac, &packet[6], sizeof(macaddr_t));
      *if_index = current_port;
      return ip_len;
    } else if (packet && hdr.caplen >= IP_OFFSET && packet[12] == 0x08 &&
               packet[13] == 0x06) {
      // ARP
      macaddr_t mac;
      memcpy(mac, &packet[22], sizeof(macaddr_t));
      in_addr_t ip;
      memcpy(&ip, &packet[28], sizeof(in_addr_t));
      memcpy(arp_table[std::pair<in_addr_t, int>(ip, current_port)], mac,
             sizeof(macaddr_t));
      if (debugEnabled) {
        fprintf(stderr, "HAL_ReceiveIPPacket: learned MAC address of %s\n",
                inet_ntoa(in_addr{ip}));
      }
      continue;
    }

    current_port = (current_port + 1) % N_IFACE_ON_BOARD;
  }
  return 0;
}

int HAL_SendIPPacket(int if_index, uint8_t *buffer, size_t length,
                     macaddr_t src_mac, macaddr_t dst_mac) {
  if (!inited) {
    return HAL_ERR_CALLED_BEFORE_INIT;
  }
  if (if_index >= N_IFACE_ON_BOARD || if_index < 0) {
    return HAL_ERR_INVALID_PARAMETER;
  }
  if (!pcap_out_handles[if_index]) {
    return HAL_ERR_IFACE_NOT_EXIST;
  }
  uint8_t *eth_buffer = (uint8_t *)malloc(length + IP_OFFSET);
  memcpy(eth_buffer, dst_mac, sizeof(macaddr_t));
  memcpy(&eth_buffer[6], src_mac, sizeof(macaddr_t));
  // IPv4
  eth_buffer[12] = 0x08;
  eth_buffer[13] = 0x00;
  memcpy(&eth_buffer[IP_OFFSET], buffer, length);
  if (pcap_inject(pcap_out_handles[if_index], eth_buffer, length + IP_OFFSET) >=
      0) {
    free(eth_buffer);
    return 0;
  } else {
    if (debugEnabled) {
      fprintf(stderr, "HAL_SendIPPacket: pcap_inject failed with %s\n",
              pcap_geterr(pcap_out_handles[if_index]));
    }
    free(eth_buffer);
    return HAL_ERR_UNKNOWN;
  }
}
}