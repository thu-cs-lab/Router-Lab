#include "lookup.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

void update(bool insert, const RoutingTableEntry entry) {
  // TODO
}

bool prefix_query(const in6_addr addr, in6_addr *nexthop, uint32_t *if_index) {
  // TODO
  return false;
}

int mask_to_len(const in6_addr mask) {
  // TODO
  return -1;
}

in6_addr len_to_mask(int len) {
  // TODO
  return {};
}
