#ifndef __LOOKUP_H__
#define __LOOKUP_H__

#include "common.h"
#include <netinet/in.h>
#include <stdint.h>

/*
  表示路由表的一项。
  保证 addr 和 len 构成合法的网络前缀。
  当 nexthop 为零时这是一条直连路由。
  你可以在全局变量中把路由表以一定的数据结构格式保存下来。
*/
/*
  Indicates an entry in the routing table.
  Ensures that addr and len form a legal network prefix.
  This is a directly connected route when nexthop is zero.
  You can keep the routing table in a certain data structure in a global
  variable.
*/
typedef struct {
  // 匹配的 IPv6 地址前缀
  // Matching IPv6 address prefix
  in6_addr addr;
  // 前缀长度
  // Prefix length
  uint32_t len;
  // 出端口编号
  // Egress port number
  uint32_t if_index;
  // 下一跳的 IPv6 地址
  // IPv6 address of the next hop
  in6_addr nexthop;
  // TODO: 为了实现 RIPng 协议，
  // 在 router 作业中需要在这里添加额外的字段保存 metric
  // TODO: In order to implement the RIPng protocol, the
  // additional fields need to be added here in the router homework to store the
  // metric
} RoutingTableEntry;

/**
 * @brief 插入/删除一条路由表表项
 * @param insert 如果要插入则为 true ，要删除则为 false
 * @param entry 要插入/删除的表项
 *
 * 插入时如果已经存在一条 addr 和 len 都相同的表项，则替换掉原有的。
 * 删除和更新时按照 addr 和 len **精确** 匹配。
 */
/**
 * @brief Insert/delete a routing table entry
 * @param insert true if you want to insert, false if you want to delete
 * @param entry The table entry to be inserted/deleted
 *
 * If there is already a table entry with the same addr and len when inserting,
 * replace the existing one. **Exact** match is required when deleting/updating.
 */
void update(bool insert, const RoutingTableEntry entry);

/**
 * @brief 进行一次路由表的查询，按照最长前缀匹配原则
 * @param addr 需要查询的目标地址，网络字节序
 * @param nexthop 如果查询到目标，把表项的 nexthop 写入
 * @param if_index 如果查询到目标，把表项的 if_index 写入
 * @return 查到则返回 true ，没查到则返回 false
 */
/**
 * @brief Perform a routing table lookup, following the longest prefix matching
 * principle
 * @param addr The target address to be queried, in network byte order
 * @param nexthop If a match is found, write the nexthop of the table entry
 * @param if_index If a match is found, write the if_index of the table entry
 * @return true if found, false if not found
 */
bool prefix_query(const in6_addr addr, in6_addr *nexthop, uint32_t *if_index);

/**
 * @brief 转换 mask 为前缀长度
 * @param mask 需要转换的 IPv6 mask
 * @return mask 合法则返回前缀长度，不合法则返回 -1
 */
/**
 * @brief Convert mask to prefix length
 * @param mask The IPv6 mask to be converted
 * @return returns prefix length if legal, -1 if not
 */
int mask_to_len(const in6_addr mask);

/**
 * @brief 转换前缀长度为 IPv6 mask，前缀长度范围为 [0,128]
 * @param len 需要转换的前缀长度
 * @return len 合法则返回对应的 mask，不合法则返回 0
 */
/**
 * @brief Convert prefix length to IPv6 mask, prefix length range is [0,128]
 * @param len The length of the prefix to be converted
 * @return If len is legal, return the corresponding mask, if not, return 0
 */
in6_addr len_to_mask(int len);

#endif