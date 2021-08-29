#include "router_hal.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "checksum.h"

in6_addr addrs[N_IFACE_ON_BOARD] = {0};
uint8_t packet[1024];

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
    bool correct = validateAndFillChecksum(packet, res);
    printf("%s\n", correct ? "Yes" : "No");
    for (int i = 0;i < res;i++) {
      printf("%02X", packet[i]);
    }
    printf("\n");
  }
  return 0;
}