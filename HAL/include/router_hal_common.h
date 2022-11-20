#ifndef __ROUTER_HAL_COMMON_H__
#define __ROUTER_HAL_COMMON_H__

// you should not include this file in your own code.
#include "checksum.h"
#include "common.h"
#include "eui64.h"
#include "router_hal.h"
#include <map>
#include <pcap.h>
#include <time.h>
#include <vector>

bool inited = false;
int debugEnabled = 0;
in6_addr interface_addrs[N_IFACE_ON_BOARD] = {0};
ether_addr interface_mac[N_IFACE_ON_BOARD] = {0};
in6_addr interface_link_local_addrs[N_IFACE_ON_BOARD] = {0};

std::map<std::pair<in6_addr, int>, ether_addr> ndp_table;
std::map<std::pair<in6_addr, int>, uint64_t> ndp_timer;

pcap_t *pcap_in_handles[N_IFACE_ON_BOARD];
pcap_t *pcap_out_handles[N_IFACE_ON_BOARD];

extern "C" {

uint64_t HAL_GetTicks() {
  struct timespec tp = {0};
  clock_gettime(CLOCK_MONOTONIC, &tp);
  // millisecond
  return (uint64_t)tp.tv_sec * 1000 + (uint64_t)tp.tv_nsec / 1000000;
}

int HAL_GetNeighborMacAddress(HAL_IN int if_index, HAL_IN in6_addr ip,
                              HAL_OUT ether_addr *o_mac) {
  if (!inited) {
    return HAL_ERR_CALLED_BEFORE_INIT;
  }
  if (if_index >= N_IFACE_ON_BOARD || if_index < 0) {
    return HAL_ERR_INVALID_PARAMETER;
  }

  // lookup ndp table
  auto it = ndp_table.find(std::pair<in6_addr, int>(ip, if_index));
  if (it != ndp_table.end()) {
    *o_mac = it->second;
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

    uint8_t buffer[sizeof(ether_header) + sizeof(ip6_hdr) +
                   sizeof(nd_neighbor_solicit) + sizeof(nd_opt_hdr) +
                   sizeof(ether_addr)] = {0};
    ether_header *ether = (ether_header *)&buffer;
    // dst mac
    ether_addr dst_mac;
    get_ipv6_mcast_mac(dest_ip, &dst_mac);
    memcpy(ether->ether_dhost, &dst_mac, sizeof(ether_addr));
    // src mac
    ether_addr src_mac;
    HAL_GetInterfaceMacAddress(if_index, &src_mac);
    memcpy(ether->ether_shost, &src_mac, sizeof(ether_addr));

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
                          sizeof(ether_addr));
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
        (nd_neighbor_solicit *)&buffer[sizeof(ether_header) + sizeof(ip6_hdr)];
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
        (nd_opt_hdr *)&buffer[sizeof(ether_header) + sizeof(ip6_hdr) +
                              sizeof(nd_neighbor_solicit)];
    // source link-layer address
    opt->nd_opt_type = ND_OPT_SOURCE_LINKADDR;
    // 8 bytes
    opt->nd_opt_len = 1;
    // source link layer address
    memcpy(&buffer[sizeof(ether_header) + sizeof(ip6_hdr) +
                   sizeof(nd_neighbor_solicit) + sizeof(nd_opt_hdr)],
           &interface_mac[if_index], 6);

    validateAndFillChecksum((uint8_t *)ip6,
                            sizeof(ip6_hdr) + sizeof(nd_neighbor_solicit) +
                                sizeof(nd_opt_hdr) + sizeof(ether_addr));

    pcap_inject(pcap_out_handles[if_index], buffer, sizeof(buffer));
  }
  return HAL_ERR_IP_NOT_EXIST;
}

int HAL_GetInterfaceMacAddress(HAL_IN int if_index, HAL_OUT ether_addr *o_mac) {
  if (!inited) {
    return HAL_ERR_CALLED_BEFORE_INIT;
  }
  if (if_index >= N_IFACE_ON_BOARD || if_index < 0) {
    return HAL_ERR_IFACE_NOT_EXIST;
  }

  *o_mac = interface_mac[if_index];
  return 0;
}

int HAL_ReceiveIPPacket(HAL_IN int if_index_mask, HAL_OUT uint8_t *buffer,
                        HAL_IN size_t length, HAL_OUT ether_addr *src_mac,
                        HAL_OUT ether_addr *dst_mac, HAL_IN int64_t timeout,
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
      current_port = (current_port + 1) % N_IFACE_ON_BOARD;
      continue;
    }
    std::vector<uint8_t> packet(&const_packet[0], &const_packet[hdr.caplen]);
    const_packet = nullptr;

    const ether_header *ether = (const ether_header *)&packet[0];
    if (packet.size() >= sizeof(ether_header) &&
        memcmp(ether->ether_shost, &interface_mac[current_port],
               sizeof(ether_addr)) == 0) {
      // skip outbound
      continue;
    } else if (packet.size() >= sizeof(ether_header) + sizeof(ip6_hdr) &&
               ether->ether_type == htons(0x86dd)) {
      // IPv6
      // TODO: what if len != caplen
      // Beware: might be larger than MTU because of offloading

      const ip6_hdr *ip6 = (ip6_hdr *)&packet[sizeof(ether_header)];
      if ((ip6->ip6_vfc) >> 4 != 6) {
        continue;
      }
      uint16_t plen = ntohs(ip6->ip6_plen);
      if (hdr.caplen < sizeof(ether_header) + sizeof(ip6_hdr) + plen) {
        continue;
      }
      if (ip6->ip6_nxt == IPPROTO_ICMPV6 || ip6->ip6_nxt == IPPROTO_UDP) {
        if (!validateAndFillChecksum((uint8_t *)ip6, sizeof(ip6_hdr) + plen)) {
          if (debugEnabled) {
            fprintf(stderr,
                    "HAL_ReceiveIPPacket: received wrong checksum from %s\n",
                    inet6_ntoa(ip6->ip6_src));
          }
          continue;
        }
      }

      // handle icmpv6
      if (ip6->ip6_nxt == IPPROTO_ICMPV6) {
        const icmp6_hdr *icmp6 =
            (icmp6_hdr *)&packet[sizeof(ether_header) + sizeof(ip6_hdr)];
        if (icmp6->icmp6_type == 136) {
          // neighbor advertisement
          nd_neighbor_advert *na =
              (nd_neighbor_advert
                   *)&packet[sizeof(ether_header) + sizeof(ip6_hdr)];
          ether_addr mac;
          memcpy(&mac, &ether->ether_shost, sizeof(ether_addr));
          in6_addr ip = na->nd_na_target;
          ndp_table[std::pair<in6_addr, int>(ip, current_port)] = mac;
          if (debugEnabled) {
            fprintf(stderr,
                    "HAL_ReceiveIPPacket: learned MAC address of %s is %s\n",
                    inet6_ntoa(ip), ether_ntoa(mac));
          }
          continue;
        } else if (icmp6->icmp6_type == 135) {
          // neighbor solicitation
          const nd_neighbor_solicit *ns =
              (nd_neighbor_solicit
                   *)&packet[sizeof(ether_header) + sizeof(ip6_hdr)];
          if (ns->nd_ns_target == interface_addrs[current_port] ||
              ns->nd_ns_target == eui64(interface_mac[current_port])) {

            // construct reply
            uint8_t buffer[sizeof(ether_header) + sizeof(ip6_hdr) +
                           sizeof(nd_neighbor_advert) + sizeof(nd_opt_hdr) +
                           sizeof(ether_addr)] = {0};
            ether_header *reply_ether = (ether_header *)&buffer;
            // dst mac
            memcpy(reply_ether->ether_dhost, ether->ether_shost,
                   sizeof(ether_addr));
            // src mac
            ether_addr src_mac;
            HAL_GetInterfaceMacAddress(current_port, &src_mac);
            memcpy(reply_ether->ether_shost, &src_mac, sizeof(ether_addr));

            // IPv6 ether type
            reply_ether->ether_type = htons(0x86dd);

            // IPv6 header
            ip6_hdr *reply_ip6 = (ip6_hdr *)&buffer[14];
            // flow label
            reply_ip6->ip6_flow = 0;
            // version
            reply_ip6->ip6_vfc = 6 << 4;
            // payload length
            // icmpv6 header + opt
            reply_ip6->ip6_plen =
                htons(sizeof(nd_neighbor_advert) + sizeof(nd_opt_hdr) +
                      sizeof(ether_addr));
            // next header
            reply_ip6->ip6_nxt = IPPROTO_ICMPV6;
            // hop limit
            reply_ip6->ip6_hlim = 255;
            // src ip
            reply_ip6->ip6_src = ns->nd_ns_target;
            // dst ip
            reply_ip6->ip6_dst = ip6->ip6_src;

            // ICMPv6
            nd_neighbor_advert *na =
                (nd_neighbor_advert
                     *)&buffer[sizeof(ether_header) + sizeof(ip6_hdr)];
            icmp6_hdr *reply_icmp6 = &na->nd_na_hdr;
            // type = neighbor advertisement
            reply_icmp6->icmp6_type = 136;
            // code = 0
            reply_icmp6->icmp6_code = 0;
            // data = solicited & override
            reply_icmp6->icmp6_data32[0] = htonl(0x60000000);
            // target ip
            na->nd_na_target = ns->nd_ns_target;

            // option
            nd_opt_hdr *opt =
                (nd_opt_hdr *)&buffer[sizeof(ether_header) + sizeof(ip6_hdr) +
                                      sizeof(nd_neighbor_advert)];
            // target link-layer address
            opt->nd_opt_type = ND_OPT_TARGET_LINKADDR;
            // 8 bytes
            opt->nd_opt_len = 1;
            // target link layer address
            memcpy(&buffer[sizeof(ether_header) + sizeof(ip6_hdr) +
                           sizeof(nd_neighbor_advert) + sizeof(nd_opt_hdr)],
                   &interface_mac[current_port], 6);

            validateAndFillChecksum(
                (uint8_t *)reply_ip6,
                sizeof(ip6_hdr) + sizeof(nd_neighbor_advert) +
                    sizeof(nd_opt_hdr) + sizeof(ether_addr));

            pcap_inject(pcap_out_handles[current_port], buffer, sizeof(buffer));
          }
          continue;
        }
      }

      // pass to user
      size_t ip_len = hdr.caplen - sizeof(ether_header);
      size_t real_length = length > ip_len ? ip_len : length;
      memcpy(buffer, &packet[sizeof(ether_header)], real_length);
      memcpy(dst_mac, ether->ether_dhost, sizeof(ether_addr));
      memcpy(src_mac, ether->ether_shost, sizeof(ether_addr));
      *if_index = current_port;
      return ip_len;
    }

    current_port = (current_port + 1) % N_IFACE_ON_BOARD;
    // -1 for infinity
  } while ((current_time = HAL_GetTicks()) < begin + timeout || timeout == -1);
  return 0;
}

int HAL_SendIPPacket(HAL_IN int if_index, HAL_IN uint8_t *buffer,
                     HAL_IN size_t length, HAL_IN ether_addr dst_mac) {
  if (!inited) {
    return HAL_ERR_CALLED_BEFORE_INIT;
  }
  if (if_index >= N_IFACE_ON_BOARD || if_index < 0) {
    return HAL_ERR_INVALID_PARAMETER;
  }
  if (!pcap_out_handles[if_index]) {
    return HAL_ERR_IFACE_NOT_EXIST;
  }
  uint8_t *eth_buffer = (uint8_t *)malloc(length + sizeof(ether_header));
  ether_header *ether = (ether_header *)eth_buffer;
  memcpy(&ether->ether_dhost, &dst_mac, sizeof(ether_addr));
  memcpy(&ether->ether_shost, &interface_mac[if_index], sizeof(ether_addr));
  // IPv6
  ether->ether_type = htons(0x86dd);

  memcpy(&eth_buffer[sizeof(ether_header)], buffer, length);
  if (pcap_inject(pcap_out_handles[if_index], eth_buffer,
                  length + sizeof(ether_header)) >= 0) {
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

#endif
