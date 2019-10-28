#include "router.h"
#include <stdint.h>
#include <stdlib.h>

/*
  RoutingTable Entry 的定义如下：
  typedef struct {
    uint32_t addr; // IPv4 地址
    uint32_t len; // 前缀长度
    uint32_t if_index; // 出端口编号
    uint32_t nexthop; // 下一跳的 IPv4 地址
  } RoutingTableEntry;

  约定 addr 和 nexthop 以 **小端序** 存储。
  这意味着 1.2.3.4 对应 0x01020304 而不是 0x04030201。
  保证 addr 仅最高 len 位可能出现非零。
  保证合法的表项里 nexthop 非零。
  你可以在全局变量中把路由表以一定的数据结构格式保存下来。
*/

/**
 * @brief 插入/删除一条路由表表项
 * @param entry 要插入/删除的表项
 * 
 * 其中 entry.nexthop 非零表示插入，为零表示删除。
 * 如果已经存在一条 addr 和 len 都相同的表项，则替换掉原有的。
 */
void update(RoutingTableEntry entry) {
  // TODO
}

/**
 * @brief 进行一次路由表的查询，按照最长前缀匹配原则
 * @param addr 需要查询的目标地址，小端序
 * @param if_index 如果查询到目标，把表项的 if_index 写入
 * @return 查询到的表项的 nexthop ，如果没查到则返回 0
 */
uint32_t query(uint32_t addr, uint32_t *if_index) {
  // TODO
  *if_index = 0;
  return 0;
}