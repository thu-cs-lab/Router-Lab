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

// 10.0.0.1 ~ 10.0.3.1
in_addr_t addrs[N_IFACE_ON_BOARD] = {0x0100000a, 0x0101000a, 0x0102000a,
                                     0x0103000a};

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
      for (int i = 0; i < N_IFACE_ON_BOARD;i++) {
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