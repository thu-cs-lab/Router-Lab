#ifndef __ROUTER_HAL_H__
#define __ROUTER_HAL_H__

#include <stdint.h>
#include <stdlib.h>
#ifdef ROUTER_BACKEND_LINUX
#include <arpa/inet.h>
#elif defined ROUTER_BACKEND_MACOS
#include <arpa/inet.h>
#elif defined ROUTER_BACKEND_STDIO
#include <arpa/inet.h>
#elif defined ROUTER_BACKEND_XILINX
typedef uint32_t in_addr_t;
#endif
// in_addr_t 是以大端序存储的，意味着 1.2.3.4 对应 0x04030201

#define N_IFACE_ON_BOARD 4
typedef uint8_t macaddr_t[6];

enum HAL_ERROR_NUMBER {
  HAL_ERR_INVALID_PARAMETER = -1000,
  HAL_ERR_IP_NOT_EXIST,
  HAL_ERR_IFACE_NOT_EXIST,
  HAL_ERR_CALLED_BEFORE_INIT,
  HAL_ERR_EOF,
  HAL_ERR_NOT_SUPPORTED,
  HAL_ERR_UNKNOWN,
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化，在所有其他函数调用前调用且仅调用一次
 *
 * @param debug IN，零表示关闭调试信息，非零表示输出调试信息到标准错误输出
 * @param if_addrs IN，包含 N_IFACE_ON_BOARD 个 IPv4 地址，对应每个端口的 IPv4
 * 地址
 *
 * @return int 0 表示成功，非 0 表示失败
 */
int HAL_Init(int debug, in_addr_t if_addrs[N_IFACE_ON_BOARD]);

/**
 * @brief 获取从启动到当前时刻的毫秒数
 *
 * @return uint64_t 毫秒数
 */
uint64_t HAL_GetTicks();

/**
 * @brief 从 ARP 表中查询 IPv4 对应的 MAC 地址
 *
 * 如果是表中不存在的IP，系统将自动发送 ARP
 * 报文进行查询，待对方主机回应后可重新调用本接口从表中查询 部分后端会限制发送的
 * ARP 报文数量，如每秒向同一个主机最多发送一个 ARP 报文
 *
 * @param if_index IN，接口索引号，[0, N_IFACE_ON_BOARD-1]
 * @param ip IN，要查询的 IP 地址
 * @param o_mac OUT，查询结果 MAC 地址
 * @return int 0 表示成功，非 0 为失败
 */
int HAL_ArpGetMacAddress(int if_index, in_addr_t ip, macaddr_t o_mac);

/**
 * @brief 获取网卡的 MAC 地址，如果为全 0 代表系统中不存在该网卡或者获取失败
 *
 * @param if_index IN，接口索引号，[0, N_IFACE_ON_BOARD-1]
 * @param o_mac OUT，网卡的 MAC 地址
 * @return int 0 表示成功，非 0 为失败
 */
int HAL_GetInterfaceMacAddress(int if_index, macaddr_t o_mac);

/**
 * @brief 接收一个 IPv4
 * 报文，保证不会收到自己发送的报文；请保证缓冲区大小足够大（如大于常见的
 * MTU），报文只能读取一次
 *
 * @param if_index_mask IN，接口索引号的 bitset，最低的 N_IFACE_ON_BOARD
 * 位有效，对于每一位，1 代表接收对应接口，0
 * 代表不接收；部分平台仅支持所有接口都开启接收的情况
 * @param buffer IN，接收缓冲区，由调用者分配
 * @param length IN，接收缓存区大小
 * @param src_mac OUT，IPv4 报文下层的源 MAC 地址
 * @param dst_mac OUT，IPv4 报文下层的目的 MAC 地址
 * @param timeout IN，设置接收超时时间（毫秒），-1 表示无限等待
 * @param if_index OUT，实际接收到的报文来源的接口号，不能为空指针
 * @return int >0 表示实际接收的报文长度，=0 表示超时返回，<0 表示发生错误
 */
int HAL_ReceiveIPPacket(int if_index_mask, uint8_t *buffer, size_t length,
                        macaddr_t src_mac, macaddr_t dst_mac, int64_t timeout,
                        int *if_index);

/**
 * @brief 发送一个 IP 报文，它的源 MAC 地址就是对应接口的 MAC 地址
 *
 * @param if_index IN，接口索引号，[0, N_IFACE_ON_BOARD-1]
 * @param buffer IN，发送缓冲区
 * @param length IN，待发送报文的长度
 * @param dst_mac IN，IPv4 报文下层的目的 MAC 地址
 * @return int 0 表示成功，非 0 为失败
 */
int HAL_SendIPPacket(int if_index, uint8_t *buffer, size_t length,
                     macaddr_t dst_mac);

#ifdef __cplusplus
}
#endif

#endif