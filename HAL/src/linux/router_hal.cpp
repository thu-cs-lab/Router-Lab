#include "router_hal.h"

extern "C" {
void HAL_Init() {
                         return ;
}

uint64_t HAL_GetTicks() {
                         return 0;
}

int HAL_ArpGetMacAddress(int if_index, in_addr_t ip, macaddr_t o_mac) {
                         return 0;
}

int HAL_GetInterfaceMacAddress(int if_index, macaddr_t o_mac) {
                         return 0;
}

int HAL_ReceiveIPPacket(int if_index, uint8_t *buffer, size_t length,
                        macaddr_t src_mac, macaddr_t dst_mac, int64_t timeout) {
                         return 0;
                        }

int HAL_SendIPPacket(int if_index, uint8_t *buffer, size_t length,
                     macaddr_t src_mac, macaddr_t dst_mac) {
                         return 0;
                     }
}