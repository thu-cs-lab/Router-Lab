#include "common.h"

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

bool operator<(const in6_addr &a, const in6_addr &b) {
  for (int i = 0; i < 16; i++) {
    if (a.s6_addr[i] != b.s6_addr[i]) {
      return a.s6_addr[i] < b.s6_addr[i];
    }
  }
  return false;
}

const char *inet6_ntoa(in6_addr addr) {
  static char buffer[1024];
  assert(inet_ntop(AF_INET6, &addr, buffer, sizeof(buffer)));
  return buffer;
}