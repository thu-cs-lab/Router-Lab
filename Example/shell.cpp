#include "router_hal.h"
#include <ncurses.h>
#include <readline/readline.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void printMAC(macaddr_t mac) {
  printf("%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3],
         mac[4], mac[5]);
}

// char buffer[2048];
char *buffer;
uint8_t packet[2048];
bool cont = false;

in_addr_t addrs[N_IFACE_ON_BOARD] = {0x0a000001, 0x0a000101, 0x0a000201,
                                     0x0a000301};

void interrupt(int _) {
  printf("Interrupt\n");
  cont = false;
  return;
}

int main() {
  printf("HAL init: %d\n", HAL_Init(1, addrs));
  signal(SIGINT, interrupt);
  while (buffer = readline("> ")) {
    if (strcmp(buffer, "time") == 0) {
      printf("Current tick %ld\n", HAL_GetTicks());
    } else if (strncmp(buffer, "arp", strlen("arp")) == 0) {
      int if_index;
      int ip1, ip2, ip3, ip4;
      sscanf(buffer, "arp %d %d.%d.%d.%d", &if_index, &ip1, &ip2, &ip3, &ip4);
      printf("Get arp address of %d.%d.%d.%d if %d\n", ip1, ip2, ip3, ip4,
             if_index);
      in_addr_t ip = (ip4 << 24) | (ip3 << 16) | (ip2 << 8) | ip1;
      macaddr_t mac;
      int res = HAL_ArpGetMacAddress(if_index, ip, mac);
      if (res == 0) {
        printf("Found: ");
        printMAC(mac);
        printf("\n");
      } else {
        printf("Not found: %d\n", res);
      }
    } else if (strncmp(buffer, "mac", strlen("mac")) == 0) {
      int if_index;
      sscanf(buffer, "mac %d", &if_index);
      macaddr_t mac;
      int res = HAL_GetInterfaceMacAddress(if_index, mac);
      if (res == 0) {
        printf("Found: ");
        printMAC(mac);
        printf("\n");
      } else {
        printf("Not found: %d\n", res);
      }
    } else if (strncmp(buffer, "cap", strlen("cap")) == 0) {
      int mask = (1 << N_IFACE_ON_BOARD) - 1;
      macaddr_t src_mac;
      macaddr_t dst_mac;
      int if_index;
      int res = HAL_ReceiveIPPacket(mask, packet, sizeof(packet), src_mac,
                                    dst_mac, 1000, &if_index);
      if (res > 0) {
        printf("Got IP packet of length %d from port %d\n", res, if_index);
        printf("Src MAC: ");
        printMAC(src_mac);
        printf(" Dst MAC: ");
        printMAC(dst_mac);
        printf("\nData: ");
        for (int i = 0; i < res; i++) {
          printf("%02X ", packet[i]);
        }
        printf("\n");
      } else if (res == 0) {
        printf("Timeout\n");
      } else {
        printf("Error: %d\n", res);
      }
    } else if (strncmp(buffer, "out", strlen("out")) == 0) {
      int if_index;
      sscanf(buffer, "out %d", &if_index);
      macaddr_t src_mac;
      macaddr_t dst_mac;
      int len = 64;
      for (int i = 0; i < len; i++) {
        packet[i] = rand();
      }
      int res = HAL_SendIPPacket(if_index, packet, len, src_mac, dst_mac);
      if (res == 0) {
        printf("Packet sent\n");
      } else {
        printf("Sent failed: %d\n", res);
      }
    } else if (strncmp(buffer, "loop", strlen("loop")) == 0) {
      cont = true;
      while (getch() == ERR && cont) {
        int mask = (1 << N_IFACE_ON_BOARD) - 1;
        macaddr_t src_mac;
        macaddr_t dst_mac;
        int if_index;
        int res = HAL_ReceiveIPPacket(mask, packet, sizeof(packet), src_mac,
                                      dst_mac, 1000, &if_index);
      }
    } else {
      printf("Unknown command.\n");
    }
    free(buffer);
  }
  return 0;
}