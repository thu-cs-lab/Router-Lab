#include "eui64.h"
#include <arpa/inet.h>
#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  ether_addr mac;
  uint8_t *mac_bytes = mac.ether_addr_octet;
  char buffer[1024];
  while (scanf("%" SCNx8 ":%" SCNx8 ":%" SCNx8 ":%" SCNx8 ":%" SCNx8 ":%" SCNx8,
               &mac_bytes[0], &mac_bytes[1], &mac_bytes[2], &mac_bytes[3],
               &mac_bytes[4], &mac_bytes[5]) > 0) {
    in6_addr ipv6 = eui64(mac);
    assert(inet_ntop(AF_INET6, &ipv6, buffer, sizeof(buffer)));
    printf("%s\n", buffer);
  }
  return 0;
}