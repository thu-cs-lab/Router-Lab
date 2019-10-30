#include "router_hal.h"
#include <ncurses.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printMAC(macaddr_t mac) {
  printf("%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3],
         mac[4], mac[5]);
}

// char buffer[2048];
char *buffer;
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
  printf("HAL init: %d\n", HAL_Init(1, addrs));
  for (int i = 0; i < N_IFACE_ON_BOARD;i++) {
    macaddr_t mac;
    HAL_GetInterfaceMacAddress(i, mac);
    printf("%d: %02X:%02X:%02X:%02X:%02X:%02X\n", i, mac[0], mac[1], mac[2],
           mac[3], mac[4], mac[5]);
  }
  signal(SIGINT, interrupt);
  rl_bind_key('\t', rl_insert);
  while ((buffer = readline("> "))) {
    add_history(buffer);
    if (strcmp(buffer, "time") == 0) {
      printf("Current tick %lld\n", HAL_GetTicks());
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
      macaddr_t dst_mac;
      int len = 64;
      for (int i = 0; i < len; i++) {
        packet[i] = rand();
      }
      for (int i = 0; i < sizeof(macaddr_t);i++) {
        dst_mac[i] = rand();
      }
      int res = HAL_SendIPPacket(if_index, packet, len, dst_mac);
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
        if (res < 0) {
          printf("loop failed with %d\n", res);
          break;
        }
      }
    } else if (strncmp(buffer, "quit", strlen("quit")) == 0) {
      free(buffer);
      break;
    } else {
      printf("Unknown command.\n");
      printf("Usage:\n");
      printf("\ttime: show current ticks\n");
      printf("\tarp index a.b.c.d: lookup arp\n");
      printf("\tmac index: print MAC address of interface\n");
      printf("\tcap: capture one packet\n");
      printf("\tout index: send random packet to interface\n");
      printf("\tloop: read packets until interrupted\n");
      printf("\tquit: exit shell\n");
    }
    free(buffer);
  }
  return 0;
}
