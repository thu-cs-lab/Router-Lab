#include "router.h"
#include <stdint.h>
#include <stdlib.h>

/**
 * @brief 插入/删除一条路由表表项
 * @param insert 如果要插入则为 true ，要删除则为 false
 * @param entry 要插入/删除的表项
 *
 * 插入时如果已经存在一条 addr 和 len 都相同的表项，则替换掉原有的。
 * 删除时按照 addr 和 len **精确** 匹配。
 */
void update(bool insert, RoutingTableEntry entry) {
  // TODO
}

/**
 * @brief 进行一次路由表的查询，按照最长前缀匹配原则
 * @param addr 需要查询的目标地址，网络字节序
 * @param nexthop 如果查询到目标，把表项的 nexthop 写入
 * @param if_index 如果查询到目标，把表项的 if_index 写入
 * @return 查到则返回 true ，没查到则返回 false
 */
bool prefix_query(uint32_t addr, uint32_t *nexthop, uint32_t *if_index) {
  // TODO
  *nexthop = 0;
  *if_index = 0;
  return false;
}

/**
 * @brief 转换 mask 为前缀长度
 * @param mask 需要转换的 mask，网络字节序
 * @return mask 合法则返回前缀长度，不合法则返回 -1
 */
int mask_to_len(uint32_t mask) {
  // TODO
  return -1;
}

/**
 * @brief 转换前缀长度为 mask，前缀长度范围为 [0,32]
 * @param len 需要转换的前缀长度
 * @return len 合法则返回对应的网络字节序的 mask，不合法则返回 0
 */
uint32_t len_to_mask(int len) {
  // TODO
  return 0;
}