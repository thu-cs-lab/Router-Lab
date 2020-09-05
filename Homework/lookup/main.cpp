#include "router.h"
#include "router_hal.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

extern void update(bool insert, RoutingTableEntry entry);
extern bool prefix_query(uint32_t addr, uint32_t *nexthop, uint32_t *if_index);
char buffer[1024];

int main(int argc, char *argv[]) {
  uint32_t addr, len, if_index, nexthop;
  char tmp;
  while (fgets(buffer, sizeof(buffer), stdin)) {
    if (buffer[0] == 'I') {
      sscanf(buffer, "%c,%x,%d,%d,%x", &tmp, &addr, &len, &if_index, &nexthop);
      RoutingTableEntry entry = {
          .addr = addr, .len = len, .if_index = if_index, .nexthop = nexthop};
      update(true, entry);
    } else if (buffer[0] == 'D') {
      sscanf(buffer, "%c,%x,%d", &tmp, &addr, &len);
      RoutingTableEntry entry = {
          .addr = addr, .len = len, .if_index = 0, .nexthop = 0};
      update(false, entry);
    } else if (buffer[0] == 'Q') {
      sscanf(buffer, "%c,%x", &tmp, &addr);
      if (prefix_query(addr, &nexthop, &if_index)) {
        printf("0x%08x %d\n", nexthop, if_index);
      } else {
        printf("Not Found\n");
      }
    }
  }
  return 0;
}