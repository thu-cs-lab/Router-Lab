#include "rip.h"
#include "router.h"
#include "router_hal.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcap.h>

// taken from linux header netinet/ip.h
struct IPHeader {
#if __BYTE_ORDER == __LITTLE_ENDIAN
  unsigned int ip_hl : 4; /* header length */
  unsigned int ip_v : 4;  /* version */
#endif
#if __BYTE_ORDER == __BIG_ENDIAN
  unsigned int ip_v : 4;  /* version */
  unsigned int ip_hl : 4; /* header length */
#endif
  uint8_t ip_tos;          /* type of service */
  uint16_t ip_len;         /* total length */
  uint16_t ip_id;          /* identification */
  uint16_t ip_off;         /* fragment offset field */
  uint8_t ip_ttl;          /* time to live */
  uint8_t ip_p;            /* protocol */
  uint16_t ip_sum;         /* checksum */
  uint32_t ip_src, ip_dst; /* source and dest address */
};

// taken from linux header netinet/udp.h
struct UDPHeader {
  uint16_t uh_sport; /* source port */
  uint16_t uh_dport; /* destination port */
  uint16_t uh_ulen;  /* udp length */
  uint16_t uh_sum;   /* udp checksum */
};

// taken from linux header netinet/ip_icmp.h
#define ICMP_DEST_UNREACH 3   /* Destination Unreachable      */
#define ICMP_TIME_EXCEEDED 11 /* Time Exceeded                */
struct ICMPHeader {
  uint8_t type; /* message type */
  uint8_t code; /* type sub-code */
  uint16_t checksum;
  union {
    struct {
      uint16_t id;
      uint16_t sequence;
    } echo;           /* echo datagram */
    uint32_t gateway; /* gateway address */
  } un;
};

extern bool validateIPChecksum(uint8_t *packet, size_t len);
extern void update(bool insert, RoutingTableEntry entry);
extern bool prefix_query(uint32_t addr, uint32_t *nexthop, uint32_t *if_index);
extern bool forward(uint8_t *packet, size_t len);
extern RipErrorCode disassemble(const uint8_t *packet, uint32_t len,
                                RipPacket *output);
extern uint32_t assemble(const RipPacket *rip, uint8_t *buffer);
extern int mask_to_len(uint32_t mask);
extern uint32_t len_to_mask(int len);

uint8_t packet[2048];
uint8_t output[2048];

// for online experiment, don't change
#ifdef ROUTER_R1
// 0: 192.168.1.1
// 1: 192.168.3.1
// 2: 192.168.6.1
// 3: 192.168.7.1
const uint32_t addrs[N_IFACE_ON_BOARD] = {0x0101a8c0, 0x0103a8c0, 0x0106a8c0,
                                           0x0107a8c0};
#elif defined(ROUTER_R2)
// 0: 192.168.3.2
// 1: 192.168.4.1
// 2: 192.168.8.1
// 3: 192.168.9.1
const uint32_t addrs[N_IFACE_ON_BOARD] = {0x0203a8c0, 0x0104a8c0, 0x0108a8c0,
                                           0x0109a8c0};
#elif defined(ROUTER_R3)
// 0: 192.168.4.2
// 1: 192.168.5.2
// 2: 192.168.10.1
// 3: 192.168.11.1
const uint32_t addrs[N_IFACE_ON_BOARD] = {0x0204a8c0, 0x0205a8c0, 0x010aa8c0,
                                           0x010ba8c0};
#else

// 自己调试用，你可以按需进行修改，注意字节序
// 0: 10.0.0.1
// 1: 10.0.1.1
// 2: 10.0.2.1
// 3: 10.0.3.1
uint32_t addrs[N_IFACE_ON_BOARD] = {0x0100000a, 0x0101000a, 0x0102000a,
                                     0x0103000a};
#endif

const uint32_t RIP_MULTICAST_ADDR = 0x090000e0;
const macaddr_t RIP_MULTICAST_MAC = {0x01, 0x00, 0x5e, 0x00, 0x00, 0x09};

int main(int argc, char *argv[]) {
  // 0a. Init address
  int res = HAL_Init(1, addrs);
  if (res < 0) {
    return res;
  }

  // 0b. Add direct routes
  // For example:
  // 10.0.0.0/24 if 0
  // 10.0.1.0/24 if 1
  // 10.0.2.0/24 if 2
  // 10.0.3.0/24 if 3
  for (uint32_t i = 0; i < N_IFACE_ON_BOARD; i++) {
    RoutingTableEntry entry = {
        .addr = addrs[i] & 0x00FFFFFF, // network byte order
        .len = 24,                     // host byte order
        .if_index = i,                 // host byte order
        .nexthop = 0                   // network byte order, means direct
    };
    update(true, entry);
  }

  uint64_t last_time = 0;
  while (1) {
    uint64_t time = HAL_GetTicks();
    // the RFC says 30s interval,
    // but for faster convergence, use 5s here
    if (time > last_time + 5 * 1000) {
      // ref. RFC 2453 Section 3.8
      printf("5s Timer\n");
      // HINT: print complete routing table to stdout/stderr for debugging
      // do the mostly same thing as step 3a.3
      // except that dst_ip is RIP multicast IP 224.0.0.9
      // and dst_mac is RIP multicast MAC 01:00:5e:00:00:09
      // fill IP headers
      struct IPHeader *ip_header = (struct IPHeader *)output;
      ip_header->ip_hl = 5;
      ip_header->ip_v = 4;
      // TODO: set tos = 0, id = 0, off = 0, ttl = 1,
      // p = 17(udp) and dst(multicast addr)

      // fill UDP headers
      struct UDPHeader *udp_header = (struct UDPHeader *)&output[20];
      // src port = 520
      udp_header->uh_sport = htons(520);
      // dst port = 520
      udp_header->uh_dport = htons(520);

      // TODO: send complete routing table to every interface
      for (int i = 0; i < N_IFACE_ON_BOARD; i++) {
        // TODO: src_ip

        // construct rip response
        RipPacket resp;
        // TODO: fill resp
        // implement split horizon with poisoned reverse
        // ref. RFC 2453 Section 3.4.3
        // between 1 and 25 (inclusive) RIP entries every packet
        // ref. RFC 2453 Section 3.6

        // assemble RIP
        uint32_t rip_len = assemble(&resp, &output[20 + 8]);

        // TODO: checksum calculation for ip and udp
        // if you don't want to calculate udp checksum, set it to zero

        // send it back
        HAL_SendIPPacket(i, output, rip_len + 20 + 8, RIP_MULTICAST_MAC);
      }
      last_time = time;
    }

    int mask = (1 << N_IFACE_ON_BOARD) - 1;
    macaddr_t src_mac;
    macaddr_t dst_mac;
    int if_index;
    res = HAL_ReceiveIPPacket(mask, packet, sizeof(packet), src_mac, dst_mac,
                              1000, &if_index);
    if (res == HAL_ERR_EOF) {
      break;
    } else if (res < 0) {
      return res;
    } else if (res == 0) {
      // Timeout
      continue;
    } else if (res > sizeof(packet)) {
      // packet is truncated, ignore it
      continue;
    }

    // 1. validate
    if (!validateIPChecksum(packet, res)) {
      printf("Invalid IP Checksum\n");
      // drop if ip checksum invalid
      continue;
    }
    uint32_t src_addr, dst_addr;
    // TODO: extract src_addr and dst_addr from packet (big endian)

    // 2. check whether dst is me
    bool dst_is_me = false;
    for (int i = 0; i < N_IFACE_ON_BOARD; i++) {
      if (memcmp(&dst_addr, &addrs[i], sizeof(uint32_t)) == 0) {
        dst_is_me = true;
        break;
      }
    }
    // TODO: handle rip multicast address(224.0.0.9)

    if (dst_is_me) {
      // 3a.1
      RipPacket rip;
      // check and validate
      RipErrorCode err = disassemble(packet, res, &rip);
      if (err == SUCCESS) {
        if (rip.command == 1) {
          // 3a.3 request, ref. RFC 2453 Section 3.9.1
          // only need to respond to whole table requests in the lab

          // fill IP headers
          struct IPHeader *ip_header = (struct IPHeader *)output;
          ip_header->ip_hl = 5;
          ip_header->ip_v = 4;
          // TODO: set tos = 0, id = 0, off = 0, ttl = 1, p = 17(udp), dst and
          // src

          // fill UDP headers
          struct UDPHeader *udp_header = (struct UDPHeader *)&output[20];
          // src port = 520
          udp_header->uh_sport = htons(520);
          // dst port = 520
          udp_header->uh_dport = htons(520);
          // TODO: udp length

          RipPacket resp;
          // TODO: fill resp
          // implement split horizon with poisoned reverse
          // ref. RFC 2453 Section 3.4.3
          // between 1 and 25 (inclusive) RIP entries every packet
          // ref. RFC 2453 Section 3.6

          // assemble RIP
          uint32_t rip_len = assemble(&resp, &output[20 + 8]);

          // TODO: checksum calculation for ip and udp
          // if you don't want to calculate udp checksum, set it to zero

          // send it back
          HAL_SendIPPacket(if_index, output, rip_len + 20 + 8, src_mac);
        } else {
          // 3a.2 response, ref. RFC 2453 Section 3.9.2
          // TODO: update routing table
          // new metric = ?
          // update metric, if_index, nexthop
          // HINT: handle nexthop = 0 case
          // HINT: read RFC 2453 Section 3.4 Distance Vector Algorithms 4.ne
          // HINT: what is missing from RoutingTableEntry?
          // you might want to use `prefix_query` and `update`, but beware of
          // the difference between exact match and longest prefix match.
          // optional: triggered updates ref. RFC 2453 Section 3.10.1
        }
      } else if (err == RipErrorCode::ERR_IP_PROTO_NOT_UDP ||
                 err == RipErrorCode::ERR_BAD_UDP_PORT) {
        // not a rip packet
        // handle icmp echo request packet
        // TODO:
        // 1. check if ip proto is icmp
        // 2. check if icmp packet type is echo request
        if (false) {
          // construct icmp echo reply
          // see RFC792 [Page 14] Echo or Echo Reply Message
          // reply is mostly the same as request,
          // you need to:
          // 1. swap src ip addr and dst ip addr
          // 2. change icmp `type` in header
          // 3. set ttl to 64
          // 4. re-calculate icmp checksum and ip checksum
          // 5. send icmp packet
        }
      } else {
        // a bad rip packet
        // you received a malformed packet >_<
        printf("Got bad RIP packet from IP %s with error: %s\n", inet_ntoa(in_addr{src_addr}),
               rip_error_to_string(err));
      }
    } else {
      // 3b.1 dst is not me
      // check if ttl is less than or equal 1
      uint8_t ttl = packet[8];
      if (ttl <= 1) {
        // send icmp time to live exceeded to src addr
        // fill IP header
        struct IPHeader *ip_header = (struct IPHeader *)output;
        ip_header->ip_hl = 5;
        ip_header->ip_v = 4;
        // TODO: set tos = 0, id = 0, off = 0, ttl = 64, p = 1(icmp), src and
        // dst

        // fill icmp header
        // see RFC792 [Page 6] Time Exceeded Message
        struct ICMPHeader *icmp_header = (struct ICMPHeader *)&output[20];
        // icmp type = Time Exceeded
        icmp_header->type = ICMP_TIME_EXCEEDED;
        // TODO:
        // set icmp code = 0
        // fill unused fields with zero
        // append "ip header and first 8 bytes of the original payload"
        // calculate icmp checksum and ip checksum
        // send icmp packet
      } else {
        // forward
        // beware of endianness
        uint32_t nexthop, dest_if;
        if (prefix_query(dst_addr, &nexthop, &dest_if)) {
          // found
          macaddr_t dest_mac;
          // direct routing means that destination ip is directly accessible
          if (nexthop == 0) {
            nexthop = dst_addr;
          }
          if (HAL_ArpGetMacAddress(dest_if, nexthop, dest_mac) == 0) {
            // found
            memcpy(output, packet, res);
            // update ttl and checksum
            forward(output, res);
            HAL_SendIPPacket(dest_if, output, res, dest_mac);
          } else {
            // not found
            // you can drop it
            printf("Nexthop ip %s is not found in ARP table\n",
                   inet_ntoa(in_addr{nexthop}));
          }
        } else {
          // not found
          // send ICMP Destination Network Unreachable
          printf("Destination IP %s not found in routing table ",
                 inet_ntoa(in_addr{dst_addr}));
          printf("and source IP is %s\n",
                 inet_ntoa(in_addr{src_addr}));
          // send icmp destination net unreachable to src addr
          // fill IP header
          struct IPHeader *ip_header = (struct IPHeader *)output;
          ip_header->ip_hl = 5;
          ip_header->ip_v = 4;
          // TODO: set tos = 0, id = 0, off = 0, ttl = 64, p = 1(icmp), src and
          // dst

          // fill icmp header
          // see RFC792 [Page 4] Destination Unreachable Message
          struct ICMPHeader *icmp_header = (struct ICMPHeader *)&output[20];
          // icmp type = Destination Unreachable
          icmp_header->type = ICMP_DEST_UNREACH;
          // TODO:
          // icmp code = Destination Network Unreachable
          // fill unused fields with zero
          // append "ip header and first 8 bytes of the original payload"
          // calculate icmp checksum and ip checksum
          // send icmp packet
        }
      }
    }
  }
  return 0;
}
