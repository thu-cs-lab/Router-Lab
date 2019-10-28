#include "router_hal.h"
#include "rip.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

extern bool disassemble(const uint8_t *packet, uint32_t len, RipPacket *output);
extern uint32_t assemble(const RipPacket *rip, uint8_t *buffer);
uint8_t buffer[1024];
uint8_t packet[2048];
RipPacket rip;
in_addr_t addrs[N_IFACE_ON_BOARD] = {0};

int main(int argc, char *argv[]) {
  int res = HAL_Init(0, addrs);
  if (res < 0) {
    return res;
  }
  while (1) {
    int mask = (1 << N_IFACE_ON_BOARD) - 1;
    macaddr_t src_mac;
    macaddr_t dst_mac;
    int if_index;
    res = HAL_ReceiveIPPacket(mask, packet, sizeof(packet), src_mac,
                                  dst_mac, -1, &if_index);
    if (res == HAL_ERR_EOF) {
      break;
    } else if (res < 0) {
      return res;
    }
    if (disassemble(packet, res, &rip)) {
      printf("Valid %d %d\n", rip.numEntries, rip.command);
      for (int i = 0;i < rip.numEntries;i++) {
        printf("%08x %08x %08x %08x\n", rip.entries[i].addr, rip.entries[i].mask, rip.entries[i].nexthop, rip.entries[i].metric);
      }
      uint32_t len = assemble(&rip, buffer);
      for (uint32_t i = 0;i < len;i++) {
        printf("%02x ", buffer[i]);
      }
      printf("\n");
    } else {
      printf("Invalid\n");
    }
  }
  return 0;
}