#include "router_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printMAC(macaddr_t mac) {
  printf("%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3],
         mac[4], mac[5]);
}

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

void interrupt(int _) {
  printf("Interrupt\n");
  cont = false;
  return;
}

int main() {
  fprintf(stderr, "HAL init: %d\n", HAL_Init(1, addrs));
  for (int i = 0; i < N_IFACE_ON_BOARD; i++) {
    macaddr_t mac;
    HAL_GetInterfaceMacAddress(i, mac);
    fprintf(stderr, "%d: %02X:%02X:%02X:%02X:%02X:%02X\n", i, mac[0], mac[1],
            mac[2], mac[3], mac[4], mac[5]);
  }

  while (1) {
    int mask = (1 << N_IFACE_ON_BOARD) - 1;
    macaddr_t src_mac;
    macaddr_t dst_mac;
    int if_index;
    int res = HAL_ReceiveIPPacket(mask, packet, sizeof(packet), src_mac,
                                  dst_mac, 1000, &if_index);
    if (res > 0) {
      for (int i = 0; i < N_IFACE_ON_BOARD; i++) {
        HAL_SendIPPacket(i, packet, res, src_mac);
      }
    } else if (res == 0) {
      fprintf(stderr, "Timeout\n");
    } else {
      fprintf(stderr, "Error: %d\n", res);
      break;
    }
  }
  return 0;
}