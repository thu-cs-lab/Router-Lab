#include "eui64.h"
#include <arpa/inet.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char *argv[]) {
  macaddr_t mac;
  char buffer[1024];
  while (scanf("%" SCNx8 ":%" SCNx8 ":%" SCNx8 ":%" SCNx8 ":%" SCNx8 ":%" SCNx8,
               &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) > 0) {
    in6_addr ipv6 = eui64(mac);
    assert(inet_ntop(AF_INET6, &ipv6, buffer, sizeof(buffer)));
    printf("%s\n", buffer);
  }
  return 0;
}