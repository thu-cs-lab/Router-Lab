#include "checksum.h"
#include "common.h"
#include "eui64.h"
#include "lookup.h"
#include "protocol.h"
#include "router_hal.h"
#include "tftp.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

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
#elif defined(ROUTER_R2)
// 0: fd00::3:2/112
// 1: fd00::5:1/112
// 2: fd00::6:1/112
// 3: fd00::7:1/112
in6_addr addrs[N_IFACE_ON_BOARD] = {
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x03, 0x00, 0x02},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x03, 0x00, 0x01},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x06, 0x00, 0x01},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x07, 0x00, 0x01},
};
// 默认网关：fd00::3:1
in6_addr default_gateway = {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x01};
#elif defined(ROUTER_PC2)
// 0: fd00::5:1/112
// 1: fd00::6:1/112
// 2: fd00::7:1/112
// 3: fd00::8:1/112
in6_addr addrs[N_IFACE_ON_BOARD] = {
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x05, 0x00, 0x01},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x06, 0x00, 0x01},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x07, 0x00, 0x01},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x08, 0x00, 0x01},
};
// 默认网关：fd00::5:2
in6_addr default_gateway = {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x02};
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

enum TransferState {
  // 正在传输
  InTransfer,
  // 传输完毕，等待最后一个 ACK
  LastAck,
};

struct transfer {
  // 读还是写
  bool is_read;
  // 服务端 TID
  uint16_t server_tid;
  // 客户端 TID
  uint16_t client_tid;
  // 客户端 IPv6 地址
  in6_addr client_addr;
  // 用于读/写的本地文件
  FILE *fp;
  // 传输状态
  TransferState state;
  // 最后一次传输的 Block 编号
  uint16_t last_block_number;
  // 最后一次传输的数据，用于重传
  uint8_t last_block_data[512];
  // 最后一次传输的数据长度
  size_t last_block_size;
};

int main(int argc, char *argv[]) {
  // 记录当前所有的传输状态
  std::vector<transfer> transfers;

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

    if (dst_is_me) {
      // 目的地址是我，按照类型进行处理

      // 检查 checksum 是否正确
      if (ip6->ip6_nxt == IPPROTO_UDP) {
        if (!validateAndFillChecksum(packet, res)) {
          printf("Received packet with bad checksum\n");
          continue;
        }
      }

      if (ip6->ip6_nxt == IPPROTO_UDP) {
        // TODO（1 行）
        // 检查 UDP 端口，判断是否为 TFTP message
        udphdr *udp = (udphdr *)&packet[sizeof(ip6_hdr)];
        if (false) {
          // TODO（1 行）
          // 新连接
          // 判断 Opcode 是否为 RRQ 或 WRQ
          tftp_hdr *tftp =
              (tftp_hdr *)&packet[sizeof(ip6_hdr) + sizeof(udphdr)];
          uint16_t opcode = ntohs(tftp->opcode);
          if (false) {
            // TODO（6 行）
            // 解析 Filename（文件名）和 Mode（传输模式）
            char file_name[1024];

            // 生成一个新的传输状态，将会插入到 transfers 数组中
            struct transfer new_transfer;

            // TODO（3 行）
            // 客户端 TID 等于客户端发送的 UDP 报文的 UDP 源端口，
            // 在 49152-65535 范围中随机生成服务端 TID，
            // 记录下客户端的 IPv6 地址。
            // 此后服务端向客户端发送的 UDP 数据报，
            // 其源 UDP 端口都为服务端 TID，
            // 其目的 UDP 端口都为客户端 TID，
            // 其源 IPv6 地址为客户端发送的请求中的目的 IPv6 地址，
            // 其目的 IPv6 地址为客户端发送的请求中的源 IPv6 地址。

            if (opcode == 1) {
              // 如果操作是读取文件
              new_transfer.is_read = true;

              // TODO（1 行）
              // 尝试打开文件，判断文件是否存在
              new_transfer.fp = fopen(file_name, "rb");
              if (false) {
                // 如果文件存在，则发送文件的第一个块。

                // 从文件中读取最多 512 字节的数据
                uint8_t block[512];
                size_t block_size = fread(block, 1, 512, new_transfer.fp);

                // 把最后一次发送的块记录下来
                // 用于重传
                new_transfer.last_block_number = 1;
                memcpy(new_transfer.last_block_data, block, block_size);
                new_transfer.last_block_size = block_size;

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
                // src ip
                reply_ip6->ip6_src = ip6->ip6_dst;
                // dst ip
                reply_ip6->ip6_dst = ip6->ip6_src;

                udphdr *reply_udp = (udphdr *)&output[sizeof(ip6_hdr)];
                // src port
                reply_udp->uh_sport = htons(new_transfer.server_tid);
                // dst port
                reply_udp->uh_dport = htons(new_transfer.client_tid);

                uint8_t *reply_tftp =
                    (uint8_t *)&output[sizeof(ip6_hdr) + sizeof(udphdr)];
                uint16_t tftp_len = 0;

                // opcode = 0x03(data)
                reply_tftp[tftp_len++] = 0x00;
                reply_tftp[tftp_len++] = 0x03;

                // # block = 1
                reply_tftp[tftp_len++] = 0x00;
                reply_tftp[tftp_len++] = 0x01;

                memcpy(&reply_tftp[tftp_len], block, block_size);
                tftp_len += block_size;

                // 根据 TFTP 消息长度，计算 UDP 和 IPv6 头部中的长度字段
                uint16_t udp_len = tftp_len + sizeof(udphdr);
                uint16_t ip_len = udp_len + sizeof(ip6_hdr);
                reply_udp->uh_ulen = htons(udp_len);
                reply_ip6->ip6_plen = htons(udp_len);
                validateAndFillChecksum(output, ip_len);

                HAL_SendIPPacket(if_index, output, ip_len, src_mac);

                // 如果第一个块大小等于 512，说明还有后续的数据需要传输，
                // 进入 InTransfer 状态；
                // 如果第一个块大小已经小于 512，则进入 LastAck 状态，
                // 表示需要等待客户端发送最后一次 ACK
                if (block_size == 512) {
                  new_transfer.state = InTransfer;
                } else {
                  new_transfer.state = LastAck;
                }

                // 记录当前传输到 transfers 数组
                transfers.push_back(new_transfer);
              } else {
                // TODO（50 行）
                // 如果文件不存在，则发送一个错误响应，
                // 其 ErrorCode 为 1，ErrMsg 为 File not found。
              }

            } else if (opcode == 2) {
              // 如果操作是写入文件

              new_transfer.is_read = false;
              new_transfer.fp = fopen(file_name, "r");
              if (new_transfer.fp) {
                // TODO（50 行）
                // 文件已经存在，则发送一个错误响应，
                // 其 ErrorCode 为 6，ErrMsg 为 File already exists。
              } else {
                // 可选功能：如果文件无法写入，也汇报错误
                new_transfer.fp = fopen(file_name, "wb");
                assert(new_transfer.fp);

                // TODO（40 行）
                // 文件不存在，则发送一个 ACK（Block Number = 0），
                // 告诉客户端可以开始发送了。

                new_transfer.last_block_number = 0;
                transfers.push_back(new_transfer);
              }
            }
          }
        } else {
          tftp_hdr *tftp =
              (tftp_hdr *)&packet[sizeof(ip6_hdr) + sizeof(udphdr)];
          uint16_t opcode = ntohs(tftp->opcode);
          uint16_t block_number = ntohs(tftp->block_number);
          for (int i = 0; i < transfers.size(); i++) {
            transfer &current_transfer = transfers[i];
            // TODO（3 行）
            // 在 `transfers` 数组中找到唯一匹配的传输，满足：
            // 源 UDP 端口等于客户端 TID 且
            // 目的 UDP 端口等于服务端 TID 且
            // 源 IPv6 地址等于客户端 IPv6 地址。
            if (false) {
              if (current_transfer.is_read) {
                // TODO（1 行）
                // 如果传输的操作是读取，判断 Opcode 是否为 ACK。
                if (false) {
                  // TODO（1 行）
                  // 如果是 ACK，检查 Block 编号
                  if (false) {
                    // TODO（1 行）
                    // 如果和最后一次发送的 Block 编号相等
                    // 判断当前状态
                    if (false) {
                      // TODO（60 行）
                      // 如果是 InTransfer 状态，说明文件还没有传输完成，
                      // 则从文件中读取下一个 Block 并发送；

                      // 如果读取的字节数不足 512，则进入 LastAck 状态
                    } else if (current_transfer.state == LastAck) {
                      // 如果是 LastAck 状态，说明这次 TFTP 读取请求已经完成，
                      // 关闭文件，
                      // 从 transfers 数组中移除当前传输
                      fclose(current_transfer.fp);
                      transfers.erase(transfers.begin() + i);
                    }
                  } else {
                    // TODO（50 行）
                    // 如果和最后一次发送的 Block
                    // 不相等（例如出现了丢包或者乱序等问题），
                    // 则重新发送最后一个 Block。
                  }
                }
              } else {
                // 如果传输的操作是写入，判断 Opcode 是否为 DATA。
                if (opcode == 0x03) {
                  // TODO（1 行）
                  // 如果 Opcode 是 DATA，检查 Block 编号
                  // 如果是最后一次传输的 Block 编号加一，说明是新传输的数据
                  if (false) {
                    // TODO（50 行）
                    // 那么写入块到文件中，并发送 ACK。
                    current_transfer.last_block_number += 1;

                    // 如果块的大小小于 512，说明这是最后一个块，写入文件后，
                    // 关闭文件，发送 ACK，
                    // 从 transfers 数组中移除当前传输
                  }
                }
              }
              break;
            }
          }
        }
      }
      continue;
    } else {
      // 目标地址不是我，忽略
    }
  }
  return 0;
}
