#include "router_hal.h"
#include "xaxidma.h"
#include "xaxiethernet.h"
#include "xil_printf.h"
#include "xspi.h"
#include <stdio.h>

const int IP_OFFSET = 14 + 4;

int inited = 0;
int debugEnabled = 0;
in_addr_t interface_addrs[N_IFACE_ON_BOARD] = {0};
macaddr_t interface_mac = {2, 3, 3, 3, 3, 3};

XAxiEthernet_Config *axiEthernetConfig;
XAxiDma_Config *axiDmaConfig;
XSpi_Config *spiConfig;

XAxiEthernet axiEthernet;
XAxiDma axiDma;
XSpi spi;

XAxiDma_BdRing *rxRing;
XAxiDma_BdRing *txRing;

#define BD_COUNT 16

char rxBdSpace[XAxiDma_BdRingMemCalc(XAXIDMA_BD_MINIMUM_ALIGNMENT, BD_COUNT)]
    __attribute__((aligned(XAXIDMA_BD_MINIMUM_ALIGNMENT)))
    __attribute__((section(".physical")));
char txBdSpace[XAxiDma_BdRingMemCalc(XAXIDMA_BD_MINIMUM_ALIGNMENT, BD_COUNT)]
    __attribute__((aligned(XAXIDMA_BD_MINIMUM_ALIGNMENT)))
    __attribute__((section(".physical")));

struct EthernetFrame {
  u8 dstMAC[6];
  u8 srcMAC[6];
  u16 etherType;
  u8 data[1508];
} rxBuffers[BD_COUNT] __attribute__((section(".physical")));

void SpiWriteRegister(u8 addr, u8 data) {
  u8 writeBuffer[3];
  // write
  writeBuffer[0] = 0x40 | (addr >> 7);
  writeBuffer[1] = addr << 1;
  writeBuffer[2] = data;
  XSpi_SetSlaveSelect(&spi, 1);
  XSpi_Transfer(&spi, writeBuffer, NULL, 3);
  xil_printf("Write SPI %d = %d\r\n", addr, data);
}

int HAL_Init(int debug, in_addr_t if_addrs[N_IFACE_ON_BOARD]) {
  XAxiDma_Bd *bd;
  if (inited) {
    return 0;
  }
  debugEnabled = debug;

  axiEthernetConfig = XAxiEthernet_LookupConfig(XPAR_AXI_ETHERNET_0_DEVICE_ID);
  axiDmaConfig = XAxiDma_LookupConfig(XPAR_AXIDMA_0_DEVICE_ID);
  spiConfig = XSpi_LookupConfig(XPAR_AXI_QUAD_SPI_0_DEVICE_ID);

  XAxiDma_CfgInitialize(&axiDma, axiDmaConfig);
  XAxiEthernet_CfgInitialize(&axiEthernet, axiEthernetConfig,
                             axiEthernetConfig->BaseAddress);
  XSpi_CfgInitialize(&spi, spiConfig, spiConfig->BaseAddress);

  xil_printf("Init vlan %x\n\r", rxBdSpace);
  XSpi_SetOptions(&spi, XSP_MASTER_OPTION | XSP_MANUAL_SSELECT_OPTION);
  XSpi_Start(&spi);
  XSpi_IntrGlobalDisable(&spi);
  // P1-P4 Tag Removal
  SpiWriteRegister(16, 2);
  SpiWriteRegister(32, 2);
  SpiWriteRegister(48, 2);
  SpiWriteRegister(64, 2);
  // P5 Tag Insertion
  SpiWriteRegister(80, 4);
  // P1-P5 PVID
  SpiWriteRegister(20, 1);
  SpiWriteRegister(36, 2);
  SpiWriteRegister(52, 3);
  SpiWriteRegister(68, 4);
  SpiWriteRegister(84, 5);

  xil_printf("Init rings @ %x\r\n", rxBdSpace);
  memset(rxBdSpace, 0, sizeof(rxBdSpace));
  rxRing = XAxiDma_GetRxRing(&axiDma);
  txRing = XAxiDma_GetTxRing(&axiDma);

  XAxiDma_BdRingCreate(rxRing, (UINTPTR)rxBdSpace, (UINTPTR)rxBdSpace,
                       XAXIDMA_BD_MINIMUM_ALIGNMENT, BD_COUNT);

  print("Enable MAC\r\n");
  XAxiEthernet_SetOptions(&axiEthernet, XAE_RECEIVER_ENABLE_OPTION |
                                            XAE_TRANSMITTER_ENABLE_OPTION);
  XAxiEthernet_Start(&axiEthernet);

  print("Add buffer to ring\n\r");
  for (int i = 0; i < BD_COUNT; i++) {
    XAxiDma_BdRingAlloc(rxRing, 1, &bd);
    XAxiDma_BdSetBufAddr(bd, (UINTPTR)&rxBuffers[i]);
    XAxiDma_BdSetLength(bd, sizeof(struct EthernetFrame),
                        rxRing->MaxTransferLen);
    XAxiDma_BdSetCtrl(bd, 0);
    XAxiDma_BdRingToHw(rxRing, 1, bd);
  }
  print("Receive start\n\r");
  XAxiDma_BdRingStart(rxRing);

  memcpy(interface_addrs, if_addrs, sizeof(interface_addrs));

  inited = 1;
  return 0;
}

uint64_t HAL_GetTicks() {
  // TODO
  static uint64_t time = 0;
  return time++;
}

int HAL_ArpGetMacAddress(int if_index, in_addr_t ip, macaddr_t o_mac) {
  if (!inited) {
    return HAL_ERR_CALLED_BEFORE_INIT;
  }
  if (if_index >= N_IFACE_ON_BOARD || if_index < 0) {
    return HAL_ERR_INVALID_PARAMETER;
  }

  /*
    auto it = arp_table.find(std::pair<in_addr_t, int>(ip, if_index));
    if (it != arp_table.end()) {
      memcpy(o_mac, it->second, sizeof(macaddr_t));
      return 0;
    } else if (pcap_out_handles[if_index] &&
               arp_timer[std::pair<in_addr_t, int>(ip, if_index)] + 1000 <
                   HAL_GetTicks()) {
      arp_timer[std::pair<in_addr_t, int>(ip, if_index)] = HAL_GetTicks();
      if (debugEnabled) {
        fprintf(
            stderr,
            "HAL_ArpGetMacAddress: asking for ip address %s with arp request\n",
            inet_ntoa(in_addr{ip}));
      }
      uint8_t buffer[64] = {0};
      // dst mac
      for (int i = 0; i < 6; i++) {
        buffer[i] = 0xff;
      }
      // src mac
      macaddr_t mac;
      HAL_GetInterfaceMacAddress(if_index, mac);
      memcpy(&buffer[6], mac, sizeof(macaddr_t));
      // ARP
      buffer[12] = 0x08;
      buffer[13] = 0x06;
      // hardware type
      buffer[15] = 0x01;
      // protocol type
      buffer[16] = 0x08;
      // hardware size
      buffer[18] = 0x06;
      // protocol size
      buffer[19] = 0x04;
      // opcode
      buffer[21] = 0x01;
      // sender
      memcpy(&buffer[22], mac, sizeof(macaddr_t));
      memcpy(&buffer[28], &interface_addrs[if_index], sizeof(in_addr_t));
      // target
      memcpy(&buffer[38], &ip, sizeof(in_addr_t));

      pcap_inject(pcap_out_handles[if_index], buffer, sizeof(buffer));
    }
    return HAL_ERR_IP_NOT_EXIST;
    */
  return 0;
}

int HAL_GetInterfaceMacAddress(int if_index, macaddr_t o_mac) {
  if (!inited) {
    return HAL_ERR_CALLED_BEFORE_INIT;
  }
  if (if_index >= N_IFACE_ON_BOARD || if_index < 0) {
    return HAL_ERR_IFACE_NOT_EXIST;
  }

  memcpy(o_mac, interface_mac, sizeof(macaddr_t));
  return 0;
}

int HAL_ReceiveIPPacket(int if_index_mask, uint8_t *buffer, size_t length,
                        macaddr_t src_mac, macaddr_t dst_mac, int64_t timeout,
                        int *if_index) {
  if (!inited) {
    return HAL_ERR_CALLED_BEFORE_INIT;
  }
  if ((if_index_mask & ((1 << N_IFACE_ON_BOARD) - 1)) == 0 || timeout < 0) {
    return HAL_ERR_INVALID_PARAMETER;
  }
  if (if_index_mask != ((1 << N_IFACE_ON_BOARD) - 1)) {
    return HAL_ERR_NOT_SUPPORTED;
  }
  XAxiDma_Bd *bd;
  uint64_t begin = HAL_GetTicks();
  uint64_t current_time = 0;
  while ((current_time = HAL_GetTicks()) < begin + timeout || timeout == -1) {
    if (XAxiDma_BdRingFromHw(rxRing, 1, &bd) == 1) {
      // See AXI Ethernet Table 3-15
      u32 length = XAxiDma_BdRead(bd, XAXIDMA_BD_USR4_OFFSET) & 0xFFFF;
      u8 *data = (u8 *)XAxiDma_BdGetBufAddr(bd);
      memcpy(dst_mac, data, sizeof(macaddr_t));
      memcpy(src_mac, &data[6], sizeof(macaddr_t));
      // Vlan ID 1-4
      *if_index = data[15] - 1;

      size_t ip_len = length - IP_OFFSET;
      size_t real_length = length > ip_len ? ip_len : length;
      memcpy(buffer, &data[IP_OFFSET], real_length);

      // recycle
      u32 addr = XAxiDma_BdGetBufAddr(bd);
      u32 len = XAxiDma_BdGetLength(bd, rxRing->MaxTransferLen);
      XAxiDma_BdRingFree(rxRing, 1, bd);

      XAxiDma_BdRingAlloc(rxRing, 1, &bd);
      XAxiDma_BdSetBufAddr(bd, addr);
      XAxiDma_BdSetLength(bd, len, rxRing->MaxTransferLen);
      XAxiDma_BdRingToHw(rxRing, 1, bd);

      return real_length;
    }
  }
  return 0;
}

int HAL_SendIPPacket(int if_index, uint8_t *buffer, size_t length,
                     macaddr_t dst_mac) {
  if (!inited) {
    return HAL_ERR_CALLED_BEFORE_INIT;
  }
  if (if_index >= N_IFACE_ON_BOARD || if_index < 0) {
    return HAL_ERR_INVALID_PARAMETER;
  }
  /*
  uint8_t *eth_buffer = (uint8_t *)malloc(length + IP_OFFSET);
  memcpy(eth_buffer, dst_mac, sizeof(macaddr_t));
  memcpy(&eth_buffer[6], interface_mac[if_index], sizeof(macaddr_t));
  // IPv4
  eth_buffer[12] = 0x08;
  eth_buffer[13] = 0x00;
  memcpy(&eth_buffer[IP_OFFSET], buffer, length);
  if (pcap_inject(pcap_out_handles[if_index], eth_buffer, length + IP_OFFSET) >=
      0) {
    free(eth_buffer);
    return 0;
  } else {
    if (debugEnabled) {
      fprintf(stderr, "HAL_SendIPPacket: pcap_inject failed with %s\n",
              pcap_geterr(pcap_out_handles[if_index]));
    }
    free(eth_buffer);
    return HAL_ERR_UNKNOWN;
  }
  */
  return 0;
}
