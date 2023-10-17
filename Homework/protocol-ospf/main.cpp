#include "protocol_ospf.h"
#include "router_hal.h"
#include <arpa/inet.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t buffer[2048];
uint8_t packet[2048];
in6_addr addrs[N_IFACE_ON_BOARD] = {0};
char addr_buffer[1024];

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
    const uint8_t *lsa_start;
    int lsa_num;
    OspfErrorCode err = parse_ip(packet, res, &lsa_start, &lsa_num);
    if (err == OspfErrorCode::SUCCESS) {
      printf("Success\n");
    } else {
      printf("Error: %s\n", ospf_error_to_string(err));
      continue;
    }
    res -= (lsa_start - packet);
    for (int i = 0; i < lsa_num; i++) {
      uint16_t lsa_len;
      RouterLsa lsa;
      OspfErrorCode err = disassemble(lsa_start, res, &lsa_len, &lsa);

      if (err == OspfErrorCode::SUCCESS) {
        printf("RouterLSA: %u\n", lsa_len);
        printf("  ls_age: %u\n", lsa.ls_age);
        printf("  link_state_id: 0x%08x\n", lsa.link_state_id);
        printf("  advertising_router: 0x%08x\n", lsa.advertising_router);
        printf("  ls_sequence_number: 0x%08x\n", lsa.ls_sequence_number);
        for (auto entry: lsa.entries) {
          printf("  Entry\n");
          printf("    type: %u\n", entry.type);
          printf("    metric: %u\n", entry.metric);
          printf("    interface_id: %u\n", entry.interface_id);
          printf("    neighbor_interface_id: %u\n", entry.neighbor_interface_id);
          printf("    neighbor_router_id: 0x%08x\n", entry.neighbor_router_id);
        }
      } else if (err == ERR_LSA_NOT_ROUTER) {
        printf("Other LSA: %u\n", lsa_len);
      } else if (err == ERR_PACKET_TOO_SHORT) {
        printf("Error: %s\n", ospf_error_to_string(err));
        break;
      } else {
        printf("Error: %s\n", ospf_error_to_string(err));
      }

      res -= lsa_len;
      lsa_start += lsa_len;
    }
  }
  return 0;
}