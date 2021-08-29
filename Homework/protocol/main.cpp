#include "protocol.h"
#include "router_hal.h"
#include <arpa/inet.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t buffer[1024];
uint8_t packet[2048];
RipPacket rip;
in6_addr addrs[N_IFACE_ON_BOARD] = {0};
char addr_buffer[1024];

int main(int argc, char *argv[]) {
  int res = HAL_Init(0, addrs);
  if (res < 0) {
    return res;
  }
  while (1) {
    int mask = (1 << N_IFACE_ON_BOARD) - 1;
    ether_addr src_mac;
    ether_addr dst_mac;
    int if_index;
    res = HAL_ReceiveIPPacket(mask, packet, sizeof(packet), &src_mac, &dst_mac,
                              -1, &if_index);
    if (res == HAL_ERR_EOF) {
      break;
    } else if (res < 0) {
      return res;
    }
    RipErrorCode err = disassemble(packet, res, &rip);
    if (err == RipErrorCode::SUCCESS) {
      printf("Valid %d %d\n", rip.numEntries, rip.command);
      for (int i = 0; i < rip.numEntries; i++) {
        assert(inet_ntop(AF_INET6, &rip.entries[i].prefix_or_nh, addr_buffer,
                         sizeof(addr_buffer)));
        printf("%s %d %d %d\n", addr_buffer, htons(rip.entries[i].route_tag),
               rip.entries[i].prefix_len, rip.entries[i].metric);
      }
      uint32_t len = assemble(&rip, buffer);
      for (uint32_t i = 0; i < len; i++) {
        printf("%02x ", buffer[i]);
      }
      printf("\n");
    } else {
      printf("%s\n", rip_error_to_string(err));
    }
  }
  return 0;
}