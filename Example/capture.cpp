#include "router_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t packet[2048];
bool cont = false;

// fd00::0:1 ~ fd00::3:1
in6_addr addrs[N_IFACE_ON_BOARD] = {
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x01},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x01, 0x00, 0x01},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x02, 0x00, 0x01},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x03, 0x00, 0x01},
};

int main() {
  fprintf(stderr, "HAL init: %d\n", HAL_Init(1, addrs));
  for (int i = 0; i < N_IFACE_ON_BOARD; i++) {
    ether_addr mac;
    HAL_GetInterfaceMacAddress(i, &mac);
    fprintf(stderr, "%d: %s\n", i, ether_ntoa(mac));
  }

  while (1) {
    int mask = (1 << N_IFACE_ON_BOARD) - 1;
    ether_addr src_mac;
    ether_addr dst_mac;
    int if_index;
    int res = HAL_ReceiveIPPacket(mask, packet, sizeof(packet), &src_mac,
                                  &dst_mac, 1000, &if_index);
    if (res > 0) {
      printf("Got IP packet of length %d from port %d\n", res, if_index);
      printf("Src MAC: %s ", ether_ntoa(dst_mac));
      printf("Dst MAC: %s\n", ether_ntoa(src_mac));
      printf("\nData: ");
      for (int i = 0; i < res; i++) {
        printf("%02X ", packet[i]);
      }
      printf("\n");
    } else if (res == 0) {
      fprintf(stderr, "Timeout\n");
    } else {
      fprintf(stderr, "Error: %d\n", res);
      break;
    }
  }
  return 0;
}