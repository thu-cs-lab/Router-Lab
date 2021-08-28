#include "lookup.h"
#include <arpa/inet.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

char buffer[1024];

int main(int argc, char *argv[]) {
  uint32_t len, if_index;
  in6_addr addr, nexthop;
  char addr_buffer[128];
  char nexthop_buffer[128];
  char tmp;
  while (fgets(buffer, sizeof(buffer), stdin)) {
    if (buffer[0] == 'I') {
      sscanf(buffer, "%c%s%d%d%s", &tmp, addr_buffer, &len, &if_index,
             nexthop_buffer);
      assert(inet_pton(AF_INET6, addr_buffer, &addr) == 1);
      assert(inet_pton(AF_INET6, nexthop_buffer, &nexthop) == 1);
      assert(0 <= len && len <= 128);
      if ((len_to_mask(len) & addr) != addr ||
          mask_to_len(len_to_mask(len)) != len) {
        printf("Invalid\n");
      } else {
        printf("Valid\n");
        RoutingTableEntry entry = {
            .addr = addr, .len = len, .if_index = if_index, .nexthop = nexthop};
        update(true, entry);
      }
    } else if (buffer[0] == 'D') {
      sscanf(buffer, "%c%s%d", &tmp, addr_buffer, &len);
      assert(inet_pton(AF_INET6, addr_buffer, &addr) == 1);
      assert(0 <= len && len <= 128);
      if ((len_to_mask(len) & addr) != addr ||
          mask_to_len(len_to_mask(len)) != len) {
        printf("Invalid\n");
      } else {
        printf("Valid\n");
        RoutingTableEntry entry = {
            .addr = addr, .len = len, .if_index = 0, .nexthop = in6_addr{}};
        update(false, entry);
      }
    } else if (buffer[0] == 'Q') {
      sscanf(buffer, "%c%s", &tmp, addr_buffer);
      assert(inet_pton(AF_INET6, addr_buffer, &addr) == 1);
      if (prefix_query(addr, &nexthop, &if_index)) {
        assert(inet_ntop(AF_INET6, &nexthop, nexthop_buffer,
                         sizeof(nexthop_buffer)));
        printf("%s %d\n", nexthop_buffer, if_index);
      } else {
        printf("Not Found\n");
      }
    }
  }
  return 0;
}