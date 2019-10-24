#include <stdint.h>
typedef struct {
    uint32_t addr;
    uint32_t len;
    uint32_t if_index;
    uint32_t nexthop;
} RoutingTableEntry;