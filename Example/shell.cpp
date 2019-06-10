#include "router_hal.h"
#include <stdio.h>
#include <string.h>

void printMAC(macaddr_t mac) {
    printf("%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

char buffer[2048];
uint8_t packet[2048];

int main() {
    printf("HAL init: %d\n", HAL_Init());
    printf("> ");
    while (fgets(buffer, sizeof(buffer), stdin) > 0) {
        if (buffer[strlen(buffer)-1] == '\n') {
            buffer[strlen(buffer)-1] = 0;
        }

        if (strcmp(buffer, "time") == 0) {
            printf("Current tick %ld\n", HAL_GetTicks());
        } else if (strncmp(buffer, "arp", strlen("arp")) == 0) {
            int if_index;
            int ip1, ip2, ip3, ip4;
            sscanf(buffer, "arp %d %d.%d.%d.%d", &if_index, &ip1, &ip2, &ip3, &ip4);
            printf("Get arp address of %d.%d.%d.%d if %d\n", ip1, ip2, ip3, ip4, if_index);
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
            int res = HAL_ReceiveIPPacket(mask, packet, sizeof(packet), src_mac, dst_mac, 1000);
            if (res > 0) {
                printf("Got IP packet of length %d\n", res);
                printf("Src MAC: ");
                printMAC(src_mac);
                printf(" Dst MAC: ");
                printMAC(dst_mac);
                printf("\nData: ");
                for (int i = 0;i < res;i++) {
                    printf("%02X ", packet[i]);
                }
                printf("\n");
            } else if (res == 0) {
                printf("Timeout\n");
            } else {
                printf("Error: %d\n", res);
            }
        } else {
            printf("Unknown command.\n");
        }
        printf("> ");
    }
    return 0;
}