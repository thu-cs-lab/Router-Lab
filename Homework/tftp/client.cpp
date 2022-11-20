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
#ifdef ROUTER_PC1
// 0: fd00::1:2/112
// 1: fd00::6:1/112
// 2: fd00::7:1/112
// 3: fd00::8:1/112
in6_addr addrs[N_IFACE_ON_BOARD] = {
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x01, 0x00, 0x02},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x06, 0x00, 0x01},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x07, 0x00, 0x01},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x08, 0x00, 0x01},
};
// 默认网关：fd00::1:1
in6_addr default_gateway = {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01};
#elif defined(ROUTER_R2)
// 0: fd00::3:2/112
// 1: fd00::4:1/112
// 2: fd00::7:1/112
// 3: fd00::8:1/112
in6_addr addrs[N_IFACE_ON_BOARD] = {
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x03, 0x00, 0x02},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x04, 0x00, 0x01},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x07, 0x00, 0x01},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x08, 0x00, 0x01},
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
// 默认网关：fd00::1:1
in6_addr default_gateway = {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01};
#endif

enum TransferState {
  // 初始状态
  Initial,
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
  // 服务端 IPv6 地址
  in6_addr server_addr;
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
  // 记录当前的传输状态
  transfer current_transfer;

  // 初始化 HAL
  int res = HAL_Init(1, addrs);
  if (res < 0) {
    return res;
  }

  // 插入直连路由
  // PC1：
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
  // PC1：
  // default via fd00::1:1 if 0
  RoutingTableEntry entry = {
      .addr = in6_addr{0}, .len = 0, .if_index = 0, .nexthop = default_gateway};
  update(true, entry);

  // 解析命令行参数
  // 第一个参数（argv[1]）：get 表示从服务读取文件，put 表示向服务器写入文件
  // 第二个参数（argv[2]）：服务端的 IPv6 地址
  // 第三个参数（argv[3]）：文件名
  // 例子：client get fd00::1:1 test 表示从 fd00::1:1 获取名为 test
  // 的文件到当前目录
  if (argc != 4) {
    printf("Invalid number of arguments\n");
    return 1;
  }

  if (strcmp(argv[1], "get") == 0) {
    current_transfer.is_read = true;
    current_transfer.fp = fopen(argv[3], "wb");
  } else if (strcmp(argv[1], "put") == 0) {
    current_transfer.is_read = false;
    current_transfer.fp = fopen(argv[3], "rb");
  } else {
    printf("Unsupported operation\n");
    return 1;
  }

  // 解析服务端 IPv6 地址
  current_transfer.server_addr = inet6_pton(argv[2]);

  // 在 49152-65535 范围中随机生成客户端 TID
  current_transfer.client_tid = 49152 + (rand() % 16384);
  // 此时还不知道服务端实际的 TID，先设为 0
  current_transfer.server_tid = 0;
  // 设置初始状态
  current_transfer.state = Initial;
  current_transfer.last_block_number = 0;

  bool done = false;
  uint64_t last_time = 0;
  while (!done) {
    // 初始状态下，尝试向服务器发送 Read/Write Request
    if (current_transfer.state == Initial) {
      // 根据服务端 IPv6 地址查询路由表，获得下一跳 IPv6 地址
      in6_addr nexthop;
      uint32_t dest_if;
      assert(prefix_query(current_transfer.server_addr, &nexthop, &dest_if));
      ether_addr dest_mac;
      // 如果下一跳为全 0，表示的是直连路由，目的机器和本路由器可以直接访问
      if (nexthop == in6_addr{0}) {
        nexthop = current_transfer.server_addr;
      }
      if (HAL_GetNeighborMacAddress(dest_if, nexthop, &dest_mac) == 0) {
        // 找到了下一跳 MAC 地址

        // 限制发送速度，每 1s 重试一次
        if (HAL_GetTicks() - last_time > 1000) {
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
          reply_ip6->ip6_src = addrs[0];
          // dst ip
          reply_ip6->ip6_dst = current_transfer.server_addr;

          udphdr *reply_udp = (udphdr *)&output[sizeof(ip6_hdr)];
          // src port
          reply_udp->uh_sport = htons(current_transfer.client_tid);
          // dst port
          reply_udp->uh_dport = htons(69);

          uint8_t *reply_tftp =
              (uint8_t *)&output[sizeof(ip6_hdr) + sizeof(udphdr)];
          uint16_t tftp_len = 0;

          if (current_transfer.is_read) {
            // opcode = 0x01(read)
            reply_tftp[tftp_len++] = 0x00;
            reply_tftp[tftp_len++] = 0x01;
          } else {
            // opcode = 0x02(write)
            reply_tftp[tftp_len++] = 0x00;
            reply_tftp[tftp_len++] = 0x02;
          }

          // TODO（4 行）
          // 文件名字段（argv[3]）

          // TODO（4 行）
          // 传输模式字段，设为 octet

          // 根据 TFTP 消息长度，计算 UDP 和 IPv6 头部中的长度字段
          uint16_t udp_len = tftp_len + sizeof(udphdr);
          uint16_t ip_len = udp_len + sizeof(ip6_hdr);
          reply_udp->uh_ulen = htons(udp_len);
          reply_ip6->ip6_plen = htons(udp_len);
          validateAndFillChecksum(output, ip_len);

          HAL_SendIPPacket(0, output, ip_len, dest_mac);

          last_time = HAL_GetTicks();
        }
      }
    }

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
        // 检查 UDP 端口，判断目的 UDP 端口是否等于客户端 TID
        udphdr *udp = (udphdr *)&packet[sizeof(ip6_hdr)];
        if (false) {

          // 检查 UDP 端口，判断源 UDP 端口是否等于服务端 TID
          // 如果还不知道服务端 TID，即此时记录的服务端 TID 为 0
          if (current_transfer.server_tid == 0) {
            // 则设置服务端 TID 为源 UDP 端口
            current_transfer.server_tid = ntohs(udp->uh_sport);
            current_transfer.state = InTransfer;
          } else {
            // TODO（1 行）
            // 检查 UDP 端口，如果源 UDP 端口不等于服务端 TID 则忽略
            if (false) {
              continue;
            }
          }

          // TODO（1 行）
          // 判断 Opcode
          tftp_hdr *tftp =
              (tftp_hdr *)&packet[sizeof(ip6_hdr) + sizeof(udphdr)];
          uint16_t opcode = ntohs(tftp->opcode);
          uint16_t block_number = ntohs(tftp->block_number);
          if (false) {
            // 如果 Opcode 是 0x03(DATA)

            // TODO（1 行）
            // 判断 Block Number 是否等于最后一次传输的 Block Number 加一
            if (false) {
              // TODO（6 行）
              // 如果等于，则把文件内容写到文件中
              // 并更新最后一次传输的 Block Number

              uint16_t block_size = 0;

              // 如果块的大小小于 512，说明这是最后一个块，写入文件后，
              // 关闭文件，发送 ACK 后就可以退出程序
              if (block_size < 512) {
                fclose(current_transfer.fp);
                printf("Get file done\n");
                done = true;
              }
            }

            // 发送 ACK，其 Block Number 为最后一次传输的 Block Number
            // TODO（40 行）

          } else if (opcode == 4) {
            // 如果 Opcode 是 0x04(ACK)

            // TODO（1 行）
            // 判断 Block 编号
            if (false) {
              // 如果 Block 编号和最后一次传输的块编号相等
              // 说明最后一次传输的块已经传输完成

              // TODO（1 行）
              // 判断当前状态
              if (false) {
                // TODO（60 行）
                // 如果是 InTransfer 状态，说明文件还没有传输完成，
                // 则从文件中读取下一个 Block 并发送；

                // 如果读取的字节数不足 512，则进入 LastAck 状态

              } else if (current_transfer.state == LastAck) {
                // 收到最后一个 ACK，说明文件传输完成
                printf("Put file done\n");
                done = true;
              }
            } else {
              // TODO（45 行）
              // 如果 Block 编号和最后一次传输的块编号不相等
              // 说明最后一次传输的块没有传输成功
              // 重新发送最后一次传输的块
            }
          } else if (opcode == 5) {
            // 如果 Opcode 是 0x05(ERROR)
            // 输出错误信息并退出
            uint16_t error_code = ntohs(tftp->error_code);

            char error_message[1024];
            strncpy(error_message,
                    (char *)&packet[sizeof(ip6_hdr) + sizeof(udphdr) + 4],
                    sizeof(error_message));

            printf("Got error #%d: %s\n", error_code, error_message);
            done = true;
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
