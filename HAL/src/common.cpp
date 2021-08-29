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

in6_addr get_solicited_node_mcast_addr(const in6_addr ip) {
  // https://datatracker.ietf.org/doc/html/rfc4291#section-2.7.1
  in6_addr res = {
      0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x01, 0xff, 0x00, 0x00, 0x00,
  };
  // replace low-order 24 bits
  memcpy(&res.s6_addr[13], &ip.s6_addr[13], 3);
  return res;
}

void get_ipv6_mcast_mac(const in6_addr mcast_ip, macaddr_t mac) {
  // https://datatracker.ietf.org/doc/html/rfc2464#section-7
  mac[0] = mac[1] = 0x33;
  memcpy(&mac[2], &mcast_ip.s6_addr[12], 4);
}