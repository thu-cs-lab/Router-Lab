#include <stdint.h>
#include <stdlib.h>
#include "eui64.h"

in6_addr eui64(const macaddr_t mac) {
  in6_addr res = {0};
  // TODO
  res.s6_addr[0] = 0xfe;
  return res;
}