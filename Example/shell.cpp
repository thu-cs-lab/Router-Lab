#include "common.h"
#include "router_hal.h"
#include <ncurses.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *buffer;
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
  printf("HAL init: %d\n", HAL_Init(1, addrs));
  for (int i = 0; i < N_IFACE_ON_BOARD; i++) {
    ether_addr mac;
    HAL_GetInterfaceMacAddress(i, &mac);
    printf("%d: %s\n", i, ether_ntoa(mac));
  }
  signal(SIGINT, interrupt);
  rl_bind_key('\t', rl_insert);
  while ((buffer = readline("> "))) {
    add_history(buffer);
    if (strcmp(buffer, "time") == 0) {
      printf("Current tick %lld\n", HAL_GetTicks());
    } else if (strncmp(buffer, "ndp", strlen("ndp")) == 0) {
      int if_index;
      int ip1, ip2, ip3, ip4;
      char addr_buffer[128];
      sscanf(buffer, "ndp %d %s", &if_index, addr_buffer);
      in6_addr ip = {};
      assert(inet_pton(AF_INET6, addr_buffer, &ip) == 1);
      printf("Get ndp address of %s if %d\n", inet6_ntoa(ip), if_index);
      ether_addr mac;
      int res = HAL_GetNeighborMacAddress(if_index, ip, &mac);
      if (res == 0) {
        printf("Found: %s\n", ether_ntoa(mac));
      } else {
        printf("Not found: %d\n", res);
      }
    } else if (strncmp(buffer, "mac", strlen("mac")) == 0) {
      int if_index;
      sscanf(buffer, "mac %d", &if_index);
      ether_addr mac;
      int res = HAL_GetInterfaceMacAddress(if_index, &mac);
      if (res == 0) {
        printf("Found: %s\n", ether_ntoa(mac));
      } else {
        printf("Not found: %d\n", res);
      }
    } else if (strncmp(buffer, "cap", strlen("cap")) == 0) {
      int mask = (1 << N_IFACE_ON_BOARD) - 1;
      ether_addr src_mac;
      ether_addr dst_mac;
      int if_index;
      int res = HAL_ReceiveIPPacket(mask, packet, sizeof(packet), &src_mac,
                                    &dst_mac, 1000, &if_index);
      if (res > 0) {
        printf("Got IP packet of length %d from port %d\n", res, if_index);
        printf("Src MAC: %s ", ether_ntoa(src_mac));
        printf(" Dst MAC: %s\n", ether_ntoa(dst_mac));
        printf("Data: ");
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
      ether_addr dst_mac;
      int len = 64;
      for (int i = 0; i < len; i++) {
        packet[i] = rand();
      }
      for (int i = 0; i < sizeof(ether_addr); i++) {
        dst_mac.ether_addr_octet[i] = rand();
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
        ether_addr src_mac;
        ether_addr dst_mac;
        int if_index;
        int res = HAL_ReceiveIPPacket(mask, packet, sizeof(packet), &src_mac,
                                      &dst_mac, 1000, &if_index);
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
      printf("\tndp index ipv6_addr: lookup by ndp\n");
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
