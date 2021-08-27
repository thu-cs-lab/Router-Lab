#include "lookup.h"
#include <stdint.h>
#include <stdlib.h>

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

in6_addr operator&(const in6_addr &a, const in6_addr &b) {
  in6_addr res;
  for (int i = 0; i < 16; i++) {
    res.s6_addr[i] = a.s6_addr[i] & b.s6_addr[i];
  }
  return res;
}

bool operator!=(const in6_addr &a, const in6_addr &b) {
  for (int i = 0; i < 16; i++) {
    if (a.s6_addr[i] != b.s6_addr[i]) {
      return true;
    }
  }
  return false;
}

bool operator==(const in6_addr &a, const in6_addr &b) { return !(a != b); }