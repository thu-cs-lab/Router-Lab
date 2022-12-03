#include "checksum.h"
#include "common.h"
#include "dhcpv6.h"
#include "eui64.h"
#include "lookup.h"
#include "protocol.h"
#include "router_hal.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t packet[2048];
uint8_t output[2048];

// for online experiment, don't change
#ifdef ROUTER_R1
// 0: fd00::1:1/112
// 1: fd00::3:1/112
// 2: fd00::6:1/112
// 3: fd00::7:1/112
in6_addr addrs[N_IFACE_ON_BOARD] = {
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x01, 0x00, 0x01},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x03, 0x00, 0x01},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x06, 0x00, 0x01},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x07, 0x00, 0x01},
};
// 默认网关：fd00::3:2
in6_addr default_gateway = {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x02};
#else

// 自己调试用，你可以按需进行修改
// 0: fd00::0:1
// 1: fd00::1:1
// 2: fd00::2:1
// 3: fd00::3:1
in6_addr addrs[N_IFACE_ON_BOARD] = {
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x01},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x01, 0x00, 0x01},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x02, 0x00, 0x01},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x03, 0x00, 0x01},
};
// 默认网关：fd00::1:2
in6_addr default_gateway = {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02};
#endif

int main(int argc, char *argv[]) {
  // 初始化 HAL
  int res = HAL_Init(1, addrs);
  if (res < 0) {
    return res;
  }

  // 插入直连路由
  // R1：
  // fd00::1:0/112 if 0
  // fd00::3:0/112 if 1
  // fd00::6:0/112 if 2
  // fd00::7:0/112 if 3
  for (uint32_t i = 0; i < N_IFACE_ON_BOARD; i++) {
    in6_addr mask = len_to_mask(112);
    RoutingTableEntry entry = {
        .addr = addrs[i] & mask,
        .len = 112,
        .if_index = i,
        .nexthop = in6_addr{0} // 全 0 表示直连路由
    };
    update(true, entry);
  }
  // 插入默认路由
  // R1：
  // default via fd00::3:2 if 1
  RoutingTableEntry entry = {
      .addr = in6_addr{0}, .len = 0, .if_index = 1, .nexthop = default_gateway};
  update(true, entry);

  while (1) {
    uint64_t time = HAL_GetTicks();

    int mask = (1 << N_IFACE_ON_BOARD) - 1;
    ether_addr src_mac;
    ether_addr dst_mac;
    int if_index;
    res = HAL_ReceiveIPPacket(mask, packet, sizeof(packet), &src_mac, &dst_mac,
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

    // 检查 IPv6 头部长度
    ip6_hdr *ip6 = (ip6_hdr *)packet;
    if (res < sizeof(ip6_hdr)) {
      printf("Received invalid ipv6 packet (%d < %d)\n", res, sizeof(ip6_hdr));
      continue;
    }
    uint16_t plen = ntohs(ip6->ip6_plen);
    if (res < plen + sizeof(ip6_hdr)) {
      printf("Received invalid ipv6 packet (%d < %d + %d)\n", res, plen,
             sizeof(ip6_hdr));
      continue;
    }

    // 检查 IPv6 头部目的地址是否为我自己
    bool dst_is_me = false;
    for (int i = 0; i < N_IFACE_ON_BOARD; i++) {
      if (memcmp(&ip6->ip6_dst, &addrs[i], sizeof(in6_addr)) == 0) {
        dst_is_me = true;
        break;
      }
    }

    // TODO（2 行）
    // 修改这个检查，当目的地址为 ICMPv6 RS 的
    // 组播目的地址（ff02::2，all-routers multicast address）或者
    // DHCPv6 Solicit 的组播目的地址（ff02::1:2）时也设置 dst_is_me 为 true。
    if (false) {
      dst_is_me = true;
    }

    if (dst_is_me) {
      // 目的地址是我，按照类型进行处理

      // 检查 checksum 是否正确
      if (ip6->ip6_nxt == IPPROTO_UDP || ip6->ip6_nxt == IPPROTO_ICMPV6) {
        if (!validateAndFillChecksum(packet, res)) {
          printf("Received packet with bad checksum\n");
          continue;
        }
      }

      if (ip6->ip6_nxt == IPPROTO_UDP) {
        // TODO（1 行）
        // 检查 UDP 端口，判断是否为 DHCPv6 message
        udphdr *udp = (udphdr *)&packet[sizeof(ip6_hdr)];
        if (false) {
          dhcpv6_hdr *dhcpv6 =
              (dhcpv6_hdr *)&packet[sizeof(ip6_hdr) + sizeof(udphdr)];
          // TODO（1 行）
          // 检查是否为 DHCPv6 Solicit 或 DHCPv6 Request
          if (false) {
            // TODO（20 行）
            // 解析 DHCPv6 头部后的 Option，找到其中的 Client Identifier
            // 和 IA_NA 中的 IAID
            // https://www.rfc-editor.org/rfc/rfc8415.html#section-21.2
            // https://www.rfc-editor.org/rfc/rfc8415.html#section-21.4

            // 构造响应的 IPv6 头部
            // IPv6 header
            ip6_hdr *reply_ip6 = (ip6_hdr *)&output[0];
            // flow label
            reply_ip6->ip6_flow = 0;
            // version
            reply_ip6->ip6_vfc = 6 << 4;
            // next header
            reply_ip6->ip6_nxt = IPPROTO_UDP;
            // hop limit
            reply_ip6->ip6_hlim = 255;
            // 源 IPv6 地址应为 Link Local 地址
            // src ip
            ether_addr mac_addr;
            HAL_GetInterfaceMacAddress(if_index, &mac_addr);
            reply_ip6->ip6_src = eui64(mac_addr);
            // dst ip
            reply_ip6->ip6_dst = ip6->ip6_src;

            udphdr *reply_udp = (udphdr *)&output[sizeof(ip6_hdr)];
            // src port
            reply_udp->uh_sport = htons(547);
            // dst port
            reply_udp->uh_dport = htons(546);

            dhcpv6_hdr *reply_dhcpv6 =
                (dhcpv6_hdr *)&output[sizeof(ip6_hdr) + sizeof(udphdr)];
            // TODO（100 行）
            // 如果是 DHCPv6 Solicit，说明客户端想要寻找一个 DHCPv6 服务器
            // 生成一个 DHCPv6 Advertise 并发送
            // 如果是 DHCPv6 Request，说明客户端想要获取动态 IPv6 地址
            // 生成一个 DHCPv6 Reply 并发送

            // 响应的 Transaction ID 与 DHCPv6 Solicit/Request 一致。

            uint16_t dhcpv6_len = 0;
            // 响应的 DHCPv6 Advertise 和 DHCPv6 Reply
            // 都包括如下的 Option：

            // 1. Server Identifier：根据本路由器在本接口上的 MAC 地址生成。
            //    - https://www.rfc-editor.org/rfc/rfc8415.html#section-21.3
            //    - Option Code: 2
            //    - Option Length: 14
            //    - DUID Type: 1 (Link-layer address plus time)
            //    - Hardware Type: 1 (Ethernet)
            //    - DUID Time: 0
            //    - Link layer address: MAC Address

            // 2. Client Identifier
            //    - https://www.rfc-editor.org/rfc/rfc8415.html#section-21.2
            //    - Option Code: 1
            //    - Option Length: 和 Solicit/Request 中的 Client Identifier
            //    一致
            //    - DUID: 和 Solicit/Request 中的 Client Identifier 一致

            // 3. Identity Association for Non-temporary
            // Address：记录服务器将会分配给客户端的 IPv6 地址。
            //    - https://www.rfc-editor.org/rfc/rfc8415.html#section-21.4
            //    - Option Code: 3
            //    - Option Length: 40
            //    - IAID: 和 Solicit/Request 中的 Identity Association for
            //    Non-temporary Address 一致
            //    - T1: 0
            //    - T2: 0
            //    - IA_NA options:
            //      - https://www.rfc-editor.org/rfc/rfc8415.html#section-21.6
            //      - Option code: 5 (IA address)
            //      - Length: 24
            //      - IPv6 Address: fd00::1:2
            //      - Preferred lifetime: 54000s
            //      - Valid lifetime: 86400s

            // 4. DNS recursive name server：包括两个 DNS 服务器地址
            // 2402:f000:1:801::8:28 和 2402:f000:1:801::8:29。
            //    - https://www.rfc-editor.org/rfc/rfc3646#section-3
            //    - Option Code: 23
            //    - Option Length: 32
            //    - DNS: 2402:f000:1:801::8:28
            //    - DNS: 2402:f000:1:801::8:29

            // 根据 DHCPv6 消息长度，计算 UDP 和 IPv6 头部中的长度字段
            uint16_t udp_len = dhcpv6_len + sizeof(dhcpv6_hdr) + sizeof(udphdr);
            uint16_t ip_len = udp_len + sizeof(ip6_hdr);
            reply_udp->uh_ulen = htons(udp_len);
            reply_ip6->ip6_plen = htons(udp_len);
            validateAndFillChecksum(output, ip_len);

            HAL_SendIPPacket(if_index, output, ip_len, src_mac);
          }
        }
      } else if (ip6->ip6_nxt == IPPROTO_ICMPV6) {
        // TODO（1 行）
        // 如果是 ICMPv6 packet
        // 检查是否是 Router Solicitation

        icmp6_hdr *icmp6 = (icmp6_hdr *)&packet[sizeof(ip6_hdr)];
        if (false) {
          // TODO（70 行）
          // 如果是 Router Solicitation，生成一个 Router Advertisement 并发送
          // 源 IPv6 地址是本路由器在本接口上的 Link Local 地址
          // 目的 IPv6 地址是 ff02::1
          // IPv6 头部的 Hop Limit 是 255
          // ICMPv6 的各字段要求如下：
          // https://www.rfc-editor.org/rfc/rfc4861#section-4.2
          // 其 Type 是 Router Advertisement，Code 是 0
          // Cur Hop Limit 设为 64
          // M（Managed address configuration）和 O（Other configuration）设为 1
          // Router Lifetime 设为 210s
          // Reachable Time 和 Retrans Timer 设为 0ms
          // 需要附上两个 ICMPv6 Option：
          // 1. Source link-layer address：内容是本路由器在本接口上的 MAC 地址
          //    - Type: 1
          //    - Length: 1
          //    - Link-layer address: MAC 地址

          // 2. MTU：1500
          //    - Type: 5
          //    - Length: 1
          //    - MTU: 1500
        }
      }
      continue;
    } else {
      // 目标地址不是我，考虑转发给下一跳
      // 检查是否是组播地址（ff00::/8），不需要转发组播分组
      if (ip6->ip6_dst.s6_addr[0] == 0xff) {
        printf("Don't forward multicast packet to %s\n",
               inet6_ntoa(ip6->ip6_dst));
        continue;
      }

      // 检查 TTL（Hop Limit）是否小于或等于 1
      uint8_t ttl = ip6->ip6_hops;
      if (ttl <= 1) {
        // 可选功能，如果实现了对调试会有帮助
        // 发送 ICMP Time Exceeded 消息
        // 将接受到的 IPv6 packet 附在 ICMPv6 头部之后。
        // 如果长度大于 1232 字节，则取前 1232 字节：
        // 1232 = IPv6 Minimum MTU(1280) - IPv6 Header(40) - ICMPv6 Header(8)
        // 意味着发送的 ICMP Time Exceeded packet 大小不大于 IPv6 Minimum MTU
        // 不会因为 MTU 问题被丢弃。
        // 详见 RFC 4443 Section 3.3 Time Exceeded Message
        // 计算 Checksum 后由自己的 IPv6 地址发送给源 IPv6 地址。
      } else {
        // 转发给下一跳
        // 按最长前缀匹配查询路由表
        in6_addr nexthop;
        uint32_t dest_if;
        if (prefix_query(ip6->ip6_dst, &nexthop, &dest_if)) {
          // 找到路由
          ether_addr dest_mac;
          // 如果下一跳为全 0，表示的是直连路由，目的机器和本路由器可以直接访问
          if (nexthop == in6_addr{0}) {
            nexthop = ip6->ip6_dst;
          }
          if (HAL_GetNeighborMacAddress(dest_if, nexthop, &dest_mac) == 0) {
            // 在 NDP 表中找到了下一跳的 MAC 地址
            // TTL-1
            ip6->ip6_hops--;

            // 转发出去
            memcpy(output, packet, res);
            HAL_SendIPPacket(dest_if, output, res, dest_mac);
          } else {
            // 没有找到下一跳的 MAC 地址
            // 本实验中可以直接丢掉，等对方回复 NDP 之后，再恢复正常转发。
            printf("Nexthop ip %s is not found in NDP table\n",
                   inet6_ntoa(nexthop));
          }
        } else {
          // 没有找到路由
          // 可选功能，如果实现了对调试会有帮助
          // 发送 ICMPv6 Destination Unreachable 消息
          // 要求与上面发送 ICMPv6 Time Exceeded 消息一致
          // Code 取 0，表示 No route to destination
          // 详见 RFC 4443 Section 3.1 Destination Unreachable Message
          // 计算 Checksum 后由自己的 IPv6 地址发送给源 IPv6 地址。

          printf("Destination IP %s not found in routing table",
                 inet6_ntoa(ip6->ip6_dst));
          printf(" and source IP is %s\n", inet6_ntoa(ip6->ip6_src));
        }
      }
    }
  }
  return 0;
}
