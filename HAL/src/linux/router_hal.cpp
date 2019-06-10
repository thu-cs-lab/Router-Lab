#include "router_hal.h"
#include <stdio.h>

#include <net/if.h>
#include <net/if_arp.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/if_packet.h>
#include <ifaddrs.h>
#include <time.h>
#include <pcap.h>

const int IP_OFFSET = 14;

bool inited = false;
int arp_fd = 0;
const char *interfaces[N_IFACE_ON_BOARD] = {
    "eth0",
    "eth1",
    "eth2",
    "eth3",
};

pcap_t *pcap_handles[N_IFACE_ON_BOARD];

extern "C" {
int HAL_Init() {
  if ((arp_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    // TODO: better error codes
    return HAL_ERR_UNKNOWN;
  }

  char error_buffer[PCAP_ERRBUF_SIZE];
  for (int i = 0;i < N_IFACE_ON_BOARD;i++) {
    pcap_handles[i] = pcap_open_live(interfaces[i], BUFSIZ, 1, 0, error_buffer);
    if (pcap_handles[i]) {
      pcap_activate(pcap_handles[i]);
    }
  }

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

  struct arpreq arp_req;
  struct sockaddr_in *sin;
  sin = (struct sockaddr_in *)&(arp_req.arp_pa);

  memset(&arp_req, 0, sizeof(arp_req));
  sin->sin_family = AF_INET;
  sin->sin_addr.s_addr = ip;
  strncpy(arp_req.arp_dev, interfaces[if_index], IF_NAMESIZE - 1);

  int ret;

  if ((ret = ioctl(arp_fd, SIOCGARP, &arp_req)) < 0) {
    // TODO: better error codes
    perror("ioctl in HAL_ArpGetMacAddress");
    return HAL_ERR_UNKNOWN;
  }

  if (arp_req.arp_flags & ATF_COM) {
    memcpy(o_mac, (unsigned char *)arp_req.arp_ha.sa_data, sizeof(macaddr_t));
    return 0;
  } else {
    return HAL_ERR_IP_NOT_EXIST;
  }
}


int HAL_GetInterfaceMacAddress(int if_index, macaddr_t o_mac) {
  if (!inited) {
    return HAL_ERR_CALLED_BEFORE_INIT;
  }

  struct ifaddrs *ifaddr, *ifa;
  if (getifaddrs(&ifaddr) < 0) {
    perror("getifaddrs in HAL_GetInterfaceMacAddress");
    return HAL_ERR_UNKNOWN;
  }

  for (ifa = ifaddr; ifa != NULL;ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL)
      continue;
    if (ifa->ifa_addr->sa_family == AF_PACKET && strcmp(ifa->ifa_name, interfaces[if_index]) == 0) {
      // found
      memcpy(o_mac, ((struct sockaddr_ll *)ifa->ifa_addr)->sll_addr, sizeof(macaddr_t));
      return 0;
    }
  }
  return HAL_ERR_IFACE_NOT_EXIST; 
}

int HAL_ReceiveIPPacket(int if_index_mask, uint8_t *buffer, size_t length,
                        macaddr_t src_mac, macaddr_t dst_mac, int64_t timeout) {
  if (!inited) {
    return HAL_ERR_CALLED_BEFORE_INIT;
  }
  if (if_index_mask & ((1 << N_IFACE_ON_BOARD) - 1) == 0) {
    return HAL_ERR_INVALID_PARAMETER;
  }

  int64_t begin = HAL_GetTicks();
  int64_t current_time = 0;
  // Round robin
  int current_port = 0;
  struct pcap_pkthdr hdr;
  while((current_time = HAL_GetTicks()) < begin + timeout) {
    if (if_index_mask & (1 << current_port) == 0 || !pcap_handles[current_port]) {
      current_port = (current_port + 1) % N_IFACE_ON_BOARD;
      continue;
    }

    int current_timeout = begin + timeout - current_time;
    // poll, but with low latency
    if (current_timeout > 10) {
      current_timeout = 10;
    }
    pcap_set_timeout(pcap_handles[current_port], current_timeout / N_IFACE_ON_BOARD);
    const uint8_t *packet = pcap_next(pcap_handles[current_port], &hdr);
    // IPv4
    if (packet && hdr.caplen >= IP_OFFSET && packet[12] == 0x08 && packet[13] == 0x00) {
      // TODO: what if len != caplen
      size_t ip_len = hdr.caplen - IP_OFFSET;
      size_t real_length = length > ip_len ? ip_len : length;
      memcpy(buffer, &packet[IP_OFFSET], real_length);
      memcpy(dst_mac, &packet[0], sizeof(macaddr_t));
      memcpy(src_mac, &packet[6], sizeof(macaddr_t));
      return ip_len;
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
  return 0;
}
}