#include "router_hal.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

extern bool forward(uint8_t *packet, size_t len);

in_addr_t addrs[N_IFACE_ON_BOARD] = {0};
uint8_t packet[1024];

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
    if (forward(packet, res)) {
      for (size_t i = 0; i < res;i++) {
        printf("%02x", packet[i]);
      }
      printf("\n");
    }
  }
  return 0;
}