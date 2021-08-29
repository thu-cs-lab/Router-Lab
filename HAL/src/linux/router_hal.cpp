#include "router_hal.h"
#include "checksum.h"
#include "common.h"
#include "eui64.h"
#include "router_hal_common.h"
#include <errno.h>
#include <ifaddrs.h>
#include <linux/if_packet.h>
#include <map>
#include <net/if.h>
#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <utility>
#include <vector>

#include "platform/standard.h"

bool inited = false;
int debugEnabled = 0;
in6_addr interface_addrs[N_IFACE_ON_BOARD] = {0};
macaddr_t interface_mac[N_IFACE_ON_BOARD] = {0};
in6_addr interface_link_local_addrs[N_IFACE_ON_BOARD] = {0};

pcap_t *pcap_in_handles[N_IFACE_ON_BOARD];
pcap_t *pcap_out_handles[N_IFACE_ON_BOARD];

std::map<std::pair<in6_addr, int>, macaddr_t> ndp_table;
std::map<std::pair<in6_addr, int>, uint64_t> ndp_timer;

extern "C" {
int HAL_Init(HAL_IN int debug, HAL_IN in6_addr if_addrs[N_IFACE_ON_BOARD]) {
  if (inited) {
    return 0;
  }
  debugEnabled = debug;

  // find matching interfaces and get their MAC address
  struct ifaddrs *ifaddr, *ifa;
  if (getifaddrs(&ifaddr) < 0) {
    if (debugEnabled) {
      fprintf(stderr, "HAL_Init: getifaddrs failed with %s\n", strerror(errno));
    }
    return HAL_ERR_UNKNOWN;
  }

  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL)
      continue;
    for (int i = 0; i < N_IFACE_ON_BOARD; i++) {
      if (ifa->ifa_addr->sa_family == AF_PACKET &&
          strcmp(ifa->ifa_name, interfaces[i]) == 0) {
        // found
        memcpy(interface_mac[i],
               ((struct sockaddr_ll *)ifa->ifa_addr)->sll_addr,
               sizeof(macaddr_t));
        memcpy(ndp_table[std::pair<in6_addr, int>(if_addrs[i], i)],
               interface_mac[i], sizeof(macaddr_t));
        if (debugEnabled) {
          fprintf(stderr, "HAL_Init: found MAC addr of interface %s\n",
                  interfaces[i]);
        }
        break;
      }
    }
  }
  freeifaddrs(ifaddr);

  // init pcap handles
  char error_buffer[PCAP_ERRBUF_SIZE];
  for (int i = 0; i < N_IFACE_ON_BOARD; i++) {
    pcap_in_handles[i] =
        pcap_open_live(interfaces[i], BUFSIZ, 1, 1, error_buffer);
    if (pcap_in_handles[i]) {
      pcap_setnonblock(pcap_in_handles[i], 1, error_buffer);
      if (debugEnabled) {
        fprintf(stderr, "HAL_Init: pcap capture enabled for %s\n",
                interfaces[i]);
      }
    } else {
      if (debugEnabled) {
        fprintf(stderr,
                "HAL_Init: pcap capture disabled for %s, either the interface "
                "does not exist or permission is denied\n",
                interfaces[i]);
      }
    }
    pcap_out_handles[i] =
        pcap_open_live(interfaces[i], BUFSIZ, 1, 0, error_buffer);
  }

  memcpy(interface_addrs, if_addrs, sizeof(interface_addrs));

  // generate link local addresses with eui64
  for (int i = 0; i < N_IFACE_ON_BOARD; i++) {
    interface_link_local_addrs[i] = eui64(interface_mac[i]);
    if (debugEnabled) {
      fprintf(stderr,
              "HAL_Init: interface %d is configured with link local addr %s\n",
              i, inet6_ntoa(interface_link_local_addrs[i]));
    }
  }

  // debug print
  if (debugEnabled) {
    for (int i = 0; i < N_IFACE_ON_BOARD; i++) {
      fprintf(stderr,
              "HAL_Init: interface %d is configured with IPv6 addr %s\n", i,
              inet6_ntoa(interface_addrs[i]));
    }
  }

  inited = true;
  return 0;
}

uint64_t HAL_GetTicks() {
  struct timespec tp = {0};
  clock_gettime(CLOCK_MONOTONIC, &tp);
  // millisecond
  return (uint64_t)tp.tv_sec * 1000 + (uint64_t)tp.tv_nsec / 1000000;
}

int HAL_GetNeighborMacAddress(HAL_IN int if_index, HAL_IN in6_addr ip,
                              HAL_OUT macaddr_t o_mac) {
  if (!inited) {
    return HAL_ERR_CALLED_BEFORE_INIT;
  }
  if (if_index >= N_IFACE_ON_BOARD || if_index < 0) {
    return HAL_ERR_INVALID_PARAMETER;
  }

  // lookup ndp table
  auto it = ndp_table.find(std::pair<in6_addr, int>(ip, if_index));
  if (it != ndp_table.end()) {
    memcpy(o_mac, it->second, sizeof(macaddr_t));
    return 0;
  } else if (pcap_out_handles[if_index] &&
             ndp_timer[std::pair<in6_addr, int>(ip, if_index)] + 1000 <
                 HAL_GetTicks()) {
    // not found, send ndp request
    // rate limit ndp request by 1 req/s
    ndp_timer[std::pair<in6_addr, int>(ip, if_index)] = HAL_GetTicks();
    if (debugEnabled) {
      fprintf(stderr,
              "HAL_GetNeighborMacAddress: asking for ip address %s with ndp "
              "request\n",
              inet6_ntoa(ip));
    }
    in6_addr dest_ip = get_solicited_node_mcast_addr(ip);

    uint8_t buffer[sizeof(ether_hdr) + sizeof(ip6_hdr) +
                   sizeof(nd_neighbor_solicit) + sizeof(nd_opt_hdr) +
                   sizeof(macaddr_t)] = {0};
    ether_hdr *ether = (ether_hdr *)&buffer;
    // dst mac
    macaddr_t dst_mac;
    get_ipv6_mcast_mac(dest_ip, dst_mac);
    memcpy(ether->dst_mac, dst_mac, sizeof(macaddr_t));
    // src mac
    macaddr_t src_mac;
    HAL_GetInterfaceMacAddress(if_index, src_mac);
    memcpy(ether->src_mac, src_mac, sizeof(macaddr_t));

    // IPv6 ether type
    ether->ether_type = htons(0x86dd);

    // IPv6 header
    ip6_hdr *ip6 = (ip6_hdr *)&buffer[14];
    // flow label
    ip6->ip6_flow = 0;
    // version
    ip6->ip6_vfc = 6 << 4;
    // payload length
    // icmpv6 header + option = 32
    ip6->ip6_plen = htons(sizeof(nd_neighbor_solicit) + sizeof(nd_opt_hdr) +
                          sizeof(macaddr_t));
    // next header
    ip6->ip6_nxt = IPPROTO_ICMPV6;
    // hop limit
    ip6->ip6_hlim = 255;
    // src ip
    ip6->ip6_src = interface_addrs[if_index];
    // dst ip
    ip6->ip6_dst = dest_ip;

    // ICMPv6
    nd_neighbor_solicit *ns =
        (nd_neighbor_solicit *)&buffer[sizeof(ether_hdr) + sizeof(ip6_hdr)];
    icmp6_hdr *icmp6 = &ns->nd_ns_hdr;
    // type = neighbor solicitation
    icmp6->icmp6_type = 135;
    // code = 0
    icmp6->icmp6_code = 0;
    // data = 0
    icmp6->icmp6_data32[0] = 0;
    // target ip
    ns->nd_ns_target = ip;

    // option
    nd_opt_hdr *opt =
        (nd_opt_hdr *)&buffer[sizeof(ether_hdr) + sizeof(ip6_hdr) +
                              sizeof(nd_neighbor_solicit)];
    // source link-layer address
    opt->nd_opt_type = ND_OPT_SOURCE_LINKADDR;
    // 8 bytes
    opt->nd_opt_len = 1;
    // source link layer address
    memcpy(&buffer[sizeof(ether_hdr) + sizeof(ip6_hdr) +
                   sizeof(nd_neighbor_solicit) + sizeof(nd_opt_hdr)],
           interface_mac[if_index], 6);

    validateAndFillChecksum((uint8_t *)ip6,
                            sizeof(ip6_hdr) + sizeof(nd_neighbor_solicit) +
                                sizeof(nd_opt_hdr) + sizeof(macaddr_t));

    pcap_inject(pcap_out_handles[if_index], buffer, sizeof(buffer));
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
  do {
    if ((if_index_mask & (1 << current_port)) == 0 ||
        !pcap_in_handles[current_port]) {
      current_port = (current_port + 1) % N_IFACE_ON_BOARD;
      continue;
    }

    const uint8_t *const_packet =
        pcap_next(pcap_in_handles[current_port], &hdr);
    if (!const_packet) {
      continue;
    }
    std::vector<uint8_t> packet(&const_packet[0], &const_packet[hdr.caplen]);
    const_packet = nullptr;

    const ether_hdr *ether = (const ether_hdr *)&packet[0];
    if (packet.size() >= sizeof(ether_hdr) &&
        memcmp(ether->src_mac, interface_mac[current_port],
               sizeof(macaddr_t)) == 0) {
      // skip outbound
      continue;
    } else if (packet.size() >= sizeof(ether_hdr) + sizeof(ip6_hdr) &&
               ether->ether_type == htons(0x86dd)) {
      // IPv6
      // TODO: what if len != caplen
      // Beware: might be larger than MTU because of offloading

      ip6_hdr *ip6 = (ip6_hdr *)&packet[sizeof(ether_hdr)];
      if ((ip6->ip6_vfc) >> 4 != 6) {
        continue;
      }
      uint16_t plen = htons(ip6->ip6_plen);
      if (hdr.caplen < sizeof(ether_hdr) + sizeof(ip6_hdr) + plen) {
        continue;
      }
      if (ip6->ip6_nxt == IPPROTO_ICMPV6 || ip6->ip6_nxt == IPPROTO_UDP) {
        if (!validateAndFillChecksum((uint8_t *)ip6, sizeof(ip6_hdr) + plen)) {
          if (debugEnabled) {
            fprintf(stderr, "HAL_ReceiveIPPacket: received wrong checksum from %s\n",
                    inet6_ntoa(ip6->ip6_src));
          }
          continue;
        }
      }

      // handle icmpv6
      if (ip6->ip6_nxt == IPPROTO_ICMPV6) {
        icmp6_hdr *icmp6 =
            (icmp6_hdr *)&packet[sizeof(ether_hdr) + sizeof(ip6_hdr)];
        if (icmp6->icmp6_type == 136) {
          // neighbor advertisement
          nd_neighbor_solicit *ns =
              (nd_neighbor_solicit
                   *)&packet[sizeof(ether_hdr) + sizeof(ip6_hdr)];
          macaddr_t mac;
          memcpy(mac, &ether->src_mac, sizeof(macaddr_t));
          in6_addr ip = ns->nd_ns_target;
          memcpy(ndp_table[std::pair<in6_addr, int>(ip, current_port)], mac,
                 sizeof(macaddr_t));
          if (debugEnabled) {
            fprintf(stderr, "HAL_ReceiveIPPacket: learned MAC address of %s\n",
                    inet6_ntoa(ip));
          }
          continue;
        }
      }

      // pass to user
      size_t ip_len = hdr.caplen - sizeof(ether_hdr);
      size_t real_length = length > ip_len ? ip_len : length;
      memcpy(buffer, &packet[sizeof(ether_hdr)], real_length);
      memcpy(dst_mac, ether->dst_mac, sizeof(macaddr_t));
      memcpy(src_mac, ether->src_mac, sizeof(macaddr_t));
      *if_index = current_port;
      return ip_len;
    }

    current_port = (current_port + 1) % N_IFACE_ON_BOARD;
    // -1 for infinity
  } while ((current_time = HAL_GetTicks()) < begin + timeout || timeout == -1);
  return 0;
}

int HAL_SendIPPacket(HAL_IN int if_index, HAL_IN uint8_t *buffer,
                     HAL_IN size_t length, HAL_IN macaddr_t dst_mac) {
  if (!inited) {
    return HAL_ERR_CALLED_BEFORE_INIT;
  }
  if (if_index >= N_IFACE_ON_BOARD || if_index < 0) {
    return HAL_ERR_INVALID_PARAMETER;
  }
  if (!pcap_out_handles[if_index]) {
    return HAL_ERR_IFACE_NOT_EXIST;
  }
  uint8_t *eth_buffer = (uint8_t *)malloc(length + sizeof(ether_hdr));
  ether_hdr *ether = (ether_hdr *)eth_buffer;
  memcpy(ether->dst_mac, dst_mac, sizeof(macaddr_t));
  memcpy(ether->src_mac, interface_mac[if_index], sizeof(macaddr_t));
  // IPv6
  ether->ether_type = htons(0x86dd);

  memcpy(&eth_buffer[sizeof(ether_hdr)], buffer, length);
  if (pcap_inject(pcap_out_handles[if_index], eth_buffer,
                  length + sizeof(ether_hdr)) >= 0) {
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
