#include <stdint.h>

// 路由表的一项
typedef struct {
    uint32_t addr; // 地址
    uint32_t len; // 前缀长度
    uint32_t if_index; // 出端口编号
    uint32_t nexthop; // 下一跳的地址，0 表示直连，注意和 RIP Entry 的 nexthop 区别： RIP 中的 nexthop = 0 表示的是源 IP 地址
    // 为了实现 RIP 协议，需要在这里添加额外的字段
} RoutingTableEntry;