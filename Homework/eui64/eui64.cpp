#include "eui64.h"
#include <stdint.h>
#include <stdlib.h>

in6_addr eui64(const ether_addr mac) {
  in6_addr res = {0};
  // TODO
  res.s6_addr[0] = 0xfe;
  return res;
}