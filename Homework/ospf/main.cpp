#include "checksum.h"
#include "common.h"
#include "eui64.h"
#include "lookup.h"
#include "protocol_ospf.h"
#include "router_hal.h"
#include <arpa/inet.h>
#include <cstdint>
#include <limits.h>
#include <map>
#include <netinet/in.h>
#include <netinet/ip6.h>
#include <queue>
#include <set>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>

uint8_t packet[2048];
uint8_t output[2048];

// for online experiment, don't change
#ifdef ROUTER_R1
uint32_t router_id = 0x01010101; // 1.1.1.1
// 0: fd00::1:1/112
// 1: fd00::3:1/112
// 2: fd00::6:1/112
// 3: fd00::7:1/112
in6_addr addrs[N_IFACE_ON_BOARD] = {
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x01, 0x00, 0x01},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x03, 0x00, 0x01},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x06, 0x00, 0x01},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x07, 0x00, 0x01},
};
#elif defined(ROUTER_R2)
uint32_t router_id = 0x01010102; // 1.1.1.2
// 0: fd00::3:2/112
// 1: fd00::4:1/112
// 2: fd00::8:1/112
// 3: fd00::9:1/112
in6_addr addrs[N_IFACE_ON_BOARD] = {
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x03, 0x00, 0x02},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x04, 0x00, 0x01},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x08, 0x00, 0x01},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x09, 0x00, 0x01},
};
#elif defined(ROUTER_R3)
uint32_t router_id = 0x01010103; // 1.1.1.3
// 0: fd00::4:2/112
// 1: fd00::5:2/112
// 2: fd00::a:1/112
// 3: fd00::b:1/112
in6_addr addrs[N_IFACE_ON_BOARD] = {
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x04, 0x00, 0x02},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x05, 0x00, 0x02},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x0a, 0x00, 0x01},
    {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x0b, 0x00, 0x01},
};
#else

uint32_t router_id = 0x01010101; // 1.1.1.1
// 自己调试用，你可以按需进行修改
// 0: fd00::0:1
// 1: fd00::1:1
// 2: fd00::2:1
// 3: fd00::3:1
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
#endif

// RFC 5340 A.3.2. The Hello Packet
// https://www.rfc-editor.org/rfc/rfc5340.html#appendix-A.3.2
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |      3        |       1       |         Packet Length         |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                         Router ID                             |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                          Area ID                              |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |          Checksum             | Instance ID   |     0         |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                        Interface ID                           |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | Rtr Priority  |             Options                           |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |        HelloInterval          |       RouterDeadInterval      |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                   Designated Router ID                        |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                Backup Designated Router ID                    |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                         Neighbor ID                           |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                        ...                                    |
//
// 使用网络字节序
struct ospf_hello {
  struct ospf_header header;
  uint32_t interface_id;                // Interface ID
  uint8_t router_priority;              // Rtr Priority
  uint8_t zero;                         // Options (high 8 bits)
  uint16_t options;                     // Options (low 16 bits)
  uint16_t hello_interval;              // HelloInterval
  uint16_t router_dead_interval;        // RouterDeadInterval
  uint32_t designated_router_id;        // Designated Router ID
  uint32_t backup_designated_router_id; // Backup Designated Router ID
  uint32_t neighbor_id[];               // Neighbor ID
};

// RFC 5340 A.3.3. The Database Description Packet
// https://www.rfc-editor.org/rfc/rfc5340.html#appendix-A.3.3
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+--+
// |      3        |       2       |        Packet Length           |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+--+
// |                           Router ID                            |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+--+
// |                             Area ID                            |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+--+
// |           Checksum            |  Instance ID  |      0         |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+--+
// |       0       |               Options                          |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+--+
// |        Interface MTU          |      0        |0|0|0|0|0|I|M|MS|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+--+
// |                    DD sequence number                          |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+--+
// |                                                                |
// +-                                                              -+
// |                                                                |
// +-                     An LSA Header                            -+
// |                                                                |
// +-                                                              -+
// |                                                                |
// +-                                                              -+
// |                                                                |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+--+
// |                       ...                                      |
//
// 使用网络字节序
struct ospf_database_description {
  struct ospf_header header;
  uint16_t zero;                       // 0 + Options (high 8 bits)
  uint16_t options;                    // Options (low 16 bits)
  uint16_t interface_mtu;              // Interface MTU
  uint16_t flags;                      // 0 + I + M + MS
  uint32_t sequence_number;            // DD sequence number
  struct ospf_lsa_header lsa_header[]; // LSA Header
};

// MTU = 1500 的情况下，OSPF DD Packet 能容纳的最大 LSA Header 数量是：
// (MTU - sizeof(struct ip6_hdr) - sizeof(header) - sizeof(zero) -
// sizeof(options) - sizeof(interface_mtu) - sizeof(flags) -
// sizeof(sequence_number)) / sizeof(struct ospf_lsa_header) = (1500 - 40 - 16 -
// 12) / 20 = 71
#define OSPF_DD_MAX_LSA_HEADER 71

// RFC 5340 A.3.4. The Link State Request Packet
// https://www.rfc-editor.org/rfc/rfc5340.html#appendix-A.3.4
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |      3        |       3       |        Packet Length          |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             Router ID                         |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             Area ID                           |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |          Checksum             |  Instance ID  |      0        |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |              0                |        LS Type                |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                         Link State ID                         |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                       Advertising Router                      |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                                 ...                           |
//
// 使用网络字节序
struct ospf_lsa_request {
  uint16_t zero;
  uint16_t ls_type;
  uint32_t link_state_id;
  uint32_t advertising_router;
};

// 使用网络字节序
struct ospf_ls_request {
  struct ospf_header header;
  // LSA Requests follow
  struct ospf_lsa_request lsa_request[];
};

// MTU = 1500 的情况下，OSPF LS Request Packet 能容纳的最大 LSA Request 数量是：
// (MTU - sizeof(ip6_hdr) - sizeof(header)) /
// sizeof(struct ospf_lsa_header) = (1500 - 40 - 16) / 12 = 120
#define OSPF_LS_REQUEST_MAX_LSA_REQUEST 120

// RFC 5340 A.3.5. The Link State Update Packet
// https://www.rfc-editor.org/rfc/rfc5340.html#appendix-A.3.5
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |      3        |       4       |         Packet Length         |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                         Router ID                             |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                          Area ID                              |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |          Checksum             |  Instance ID  |      0        |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           # LSAs                              |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                                                               |
// +-                                                            +-+
// |                            LSAs                               |
// +-                                                            +-+
// |                             ...                               |
//
// 使用网络字节序
struct ospf_ls_update {
  struct ospf_header header;
  uint32_t lsas; // # LSAs
  // LSAs follows
};

// RFC 5340 A.3.6. The Link State Acknowledgement Packet
// https://www.rfc-editor.org/rfc/rfc5340.html#appendix-A.3.6
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |      3        |       5       |        Packet Length          |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                         Router ID                             |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                          Area ID                              |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |          Checksum             |  Instance ID  |      0        |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                                                               |
// +-                                                             -+
// |                                                               |
// +-                        An LSA Header                        -+
// |                                                               |
// +-                                                             -+
// |                                                               |
// +-                                                             -+
// |                                                               |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              ...                              |
//
// 使用网络字节序
struct ospf_ls_acknowledgement {
  struct ospf_header header;
  struct ospf_lsa_header lsa_header[]; // LSA Header
};

// RFC 5340 A.4.10. Intra-Area-Prefix-LSAs
// https://www.rfc-editor.org/rfc/rfc5340.html#appendix-A.4.10
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |           LS Age              |0|0|1|            9            |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                       Link State ID                           |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                    Advertising Router                         |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                    LS Sequence Number                         |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |        LS Checksum            |             Length            |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |         # Prefixes            |     Referenced LS Type        |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                  Referenced Link State ID                     |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |               Referenced Advertising Router                   |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  PrefixLength | PrefixOptions |          Metric               |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                       Address Prefix                          |
// |                             ...                               |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             ...                               |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  PrefixLength | PrefixOptions |          Metric               |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                       Address Prefix                          |
// |                             ...                               |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// 使用网络字节序
struct ospf_intra_area_prefix_lsa_prefix_entry {
  uint8_t prefix_length;   // PrefixLength
  uint8_t prefix_options;  // PrefixOptions
  uint16_t metric;         // Metric
  in6_addr address_prefix; // Address Prefix
};

// 使用网络字节序
struct ospf_intra_area_prefix_lsa {
  struct ospf_lsa_header header;
  uint16_t prefixes;                      // # Prefixes
  uint16_t referenced_ls_type;            // Referenced LS Type
  uint32_t referenced_link_state_id;      // Referenced Link State ID
  uint32_t referenced_advertising_router; // Referenced Advertising Router
  ospf_intra_area_prefix_lsa_prefix_entry entries[];
};

// RFC 5340 A.4.2.1 LSA Type
// https://www.rfc-editor.org/rfc/rfc5340.html#appendix-A.4.2.1
enum ospf_lsa_type {
  OSPF_ROUTER_LSA = 0x2001,
  OSPF_INTRA_AREA_PREFIX_LSA = 0x2009
};

// 用主机字节序保存 Intra-Area-Prefix LSA 的地址前缀
struct IntraAreaPrefixLsaEntry {
  uint8_t prefix_length;   // Prefix Length
  uint8_t prefix_options;  // Prefix Options
  uint16_t metric;         // Metric
  in6_addr address_prefix; // Address Prefix

  int get_address_prefix_bytes() const {
    // RFC 5340 A.4.1 IPv6 Prefix Representation
    // "Address Prefix is an encoding of the prefix itself as an even multiple
    // of 32-bit words, padding with zero bits as necessary.  This encoding
    // consumes ((PrefixLength + 31) / 32) 32-bit words."
    int words = (prefix_length + 31) / 32;
    int bytes = words * 4;
    return bytes;
  }
};

// 用主机字节序保存 Intra-Area-Prefix LSA 的信息
struct IntraAreaPrefixLsa {
  uint16_t ls_age;                        // LS Age
  uint32_t link_state_id;                 // Link State ID
  uint32_t advertising_router;            // Advertising Router
  uint32_t ls_sequence_number;            // LS Sequence Number
  uint16_t referenced_ls_type;            // Referenced LS Type
  uint32_t referenced_link_state_id;      // Referenced Link State ID
  uint32_t referenced_advertising_router; // Referenced Advertising Router
  std::vector<IntraAreaPrefixLsaEntry> entries;
};

// OSPF LSA, can be one of:
// Router-LSA
// Intra-Area-Prefix-LSA
// 主机字节序
struct Lsa {
  enum ospf_lsa_type type;

  // Valid if type == OSPF_ROUTER_LSA
  RouterLsa router_lsa;

  // Valid if type == OSPF_INTRA_AREA_PREFIX_LSA
  IntraAreaPrefixLsa intra_area_perfix_lsa;

  uint32_t get_link_state_id() const {
    if (type == OSPF_ROUTER_LSA) {
      return router_lsa.link_state_id;
    } else {
      return intra_area_perfix_lsa.link_state_id;
    }
  }

  uint32_t get_advertising_router() const {
    if (type == OSPF_ROUTER_LSA) {
      return router_lsa.advertising_router;
    } else {
      return intra_area_perfix_lsa.advertising_router;
    }
  }

  uint32_t get_ls_sequence_number() const {
    if (type == OSPF_ROUTER_LSA) {
      return router_lsa.ls_sequence_number;
    } else {
      return intra_area_perfix_lsa.ls_sequence_number;
    }
  }

  uint32_t get_ls_age() const {
    if (type == OSPF_ROUTER_LSA) {
      return router_lsa.ls_age;
    } else {
      return intra_area_perfix_lsa.ls_age;
    }
  }
};

// 根据 LSA 的内容，填写 LSA Header
void ospf_fill_lsa_header(Lsa &lsa, struct ospf_lsa_header *header) {
  header->ls_type = htons(lsa.type);
  header->ls_checksum = 0;

  uint16_t length;
  if (lsa.type == OSPF_ROUTER_LSA) {
    header->ls_age = htons(lsa.router_lsa.ls_age);
    header->link_state_id = htonl(lsa.router_lsa.link_state_id);
    header->advertising_router = htonl(lsa.router_lsa.advertising_router);
    header->ls_sequence_number = htonl(lsa.router_lsa.ls_sequence_number);
    length =
        sizeof(struct ospf_router_lsa) +
        sizeof(struct ospf_router_lsa_entry) * lsa.router_lsa.entries.size();
  } else if (lsa.type == OSPF_INTRA_AREA_PREFIX_LSA) {
    header->ls_age = htons(lsa.intra_area_perfix_lsa.ls_age);
    header->link_state_id = htonl(lsa.intra_area_perfix_lsa.link_state_id);
    header->advertising_router =
        htonl(lsa.intra_area_perfix_lsa.advertising_router);
    header->ls_sequence_number =
        htonl(lsa.intra_area_perfix_lsa.ls_sequence_number);
    length = sizeof(ospf_intra_area_prefix_lsa);
    for (auto entry : lsa.intra_area_perfix_lsa.entries) {
      length += offsetof(struct ospf_intra_area_prefix_lsa_prefix_entry,
                         address_prefix) +
                entry.get_address_prefix_bytes();
    }
  } else {
    // Unreachable
    assert(false);
  }
  header->length = htons(length);
}

// 根据 LSA 的内容，填写 LSA，返回写入的 LSA 长度
uint16_t ospf_fill_lsa(Lsa &lsa, struct ospf_lsa_header *header) {
  header->ls_type = htons(lsa.type);
  header->ls_checksum = 0;

  uint16_t length;
  if (lsa.type == OSPF_ROUTER_LSA) {
    header->ls_age = htons(lsa.router_lsa.ls_age);
    header->link_state_id = htonl(lsa.router_lsa.link_state_id);
    header->advertising_router = htonl(lsa.router_lsa.advertising_router);
    header->ls_sequence_number = htonl(lsa.router_lsa.ls_sequence_number);

    ospf_router_lsa *reply_router_lsa = (ospf_router_lsa *)header;
    reply_router_lsa->flags = 0;
    reply_router_lsa->zero = 0;
    // V6(0x1) | E(0x2) | R(0x10)
    reply_router_lsa->options = htons(0x1 | 0x2 | 0x10);

    for (int k = 0; k < lsa.router_lsa.entries.size(); k++) {
      reply_router_lsa->entries[k].type = lsa.router_lsa.entries[k].type;
      reply_router_lsa->entries[k].zero = 0;
      reply_router_lsa->entries[k].metric =
          htons(lsa.router_lsa.entries[k].metric);
      reply_router_lsa->entries[k].interface_id =
          htonl(lsa.router_lsa.entries[k].interface_id);
      reply_router_lsa->entries[k].neighbor_interface_id =
          htonl(lsa.router_lsa.entries[k].neighbor_interface_id);
      reply_router_lsa->entries[k].neighbor_router_id =
          htonl(lsa.router_lsa.entries[k].neighbor_router_id);
    }
    length =
        sizeof(struct ospf_router_lsa) +
        sizeof(struct ospf_router_lsa_entry) * lsa.router_lsa.entries.size();
  } else if (lsa.type == OSPF_INTRA_AREA_PREFIX_LSA) {
    header->ls_age = htons(lsa.intra_area_perfix_lsa.ls_age);
    header->link_state_id = htonl(lsa.intra_area_perfix_lsa.link_state_id);
    header->advertising_router =
        htonl(lsa.intra_area_perfix_lsa.advertising_router);
    header->ls_sequence_number =
        htonl(lsa.intra_area_perfix_lsa.ls_sequence_number);

    ospf_intra_area_prefix_lsa *reply_intra_area_prefix_lsa =
        (ospf_intra_area_prefix_lsa *)header;
    reply_intra_area_prefix_lsa->prefixes =
        htons(lsa.intra_area_perfix_lsa.entries.size());
    reply_intra_area_prefix_lsa->referenced_ls_type =
        htons(lsa.intra_area_perfix_lsa.referenced_ls_type);
    reply_intra_area_prefix_lsa->referenced_link_state_id =
        htonl(lsa.intra_area_perfix_lsa.referenced_link_state_id);
    reply_intra_area_prefix_lsa->referenced_advertising_router =
        htonl(lsa.intra_area_perfix_lsa.referenced_advertising_router);

    length = sizeof(ospf_intra_area_prefix_lsa);

    // 遍历每个地址前缀
    for (auto entry : lsa.intra_area_perfix_lsa.entries) {
      struct ospf_intra_area_prefix_lsa_prefix_entry *reply_entry =
          (struct ospf_intra_area_prefix_lsa_prefix_entry *)&(
              ((uint8_t *)header)[length]);
      reply_entry->prefix_length = entry.prefix_length;
      reply_entry->prefix_options = entry.prefix_options;
      reply_entry->metric = htons(entry.metric);

      int bytes = entry.get_address_prefix_bytes();

      memcpy(&(((uint8_t *)
                    header)[length +
                            offsetof(
                                struct ospf_intra_area_prefix_lsa_prefix_entry,
                                address_prefix)]),
             &entry.address_prefix, bytes);

      length += offsetof(struct ospf_intra_area_prefix_lsa_prefix_entry,
                         address_prefix) +
                entry.get_address_prefix_bytes();
    }
  } else {
    // Unreachable
    assert(false);
  }
  header->length = htons(length);

  // 计算 LSA checksum
  uint16_t checksum = ospf_lsa_checksum(header, length);
  header->ls_checksum = checksum;

  return length;
}

// RFC 2328 10.1. Neighbor states
enum ospf_neighbor_state {
  // "This is the initial state of a neighbor conversation.  It indicates that
  // there has been no recent information received from the neighbor.  On NBMA
  // networks, Hello packets may still be sent to "Down" neighbors, although at
  // a reduced frequency (see Section 9.5.1)."
  NeighborDown,

  // "This state is only valid for neighbors attached to NBMA networks.  It
  // indicates that no recent information has been received from the neighbor,
  // but that a more concerted effort should be made to contact the neighbor.
  // This is done by sending the neighbor Hello packets at intervals of
  // HelloInterval (see Section 9.5.1)."
  NeighborAttempt,

  // "In this state, an Hello packet has recently been seen from the neighbor.
  // However, bidirectional communication has not yet been established with the
  // neighbor (i.e., the router itself did not appear in the neighbor's Hello
  // packet).  All  neighbors in this state (or higher) are listed in the Hello
  // packets sent from the associated interface."
  NeighborInit,

  // "In this state, communication between the two routers is bidirectional.
  // This has been assured by the operation of the Hello Protocol.  This is the
  // most advanced state short of beginning adjacency establishment.  The
  // (Backup) Designated Router is selected from the set of neighbors in state
  // 2-Way or greater."
  Neighbor2Way,

  // "This is the first step in creating an adjacency between the two
  // neighboring routers.  The goal of this step is to decide which router is
  // the master, and to decide upon the initial DD sequence number.  Neighbor
  // conversations in this state or greater are called adjacencies."
  NeighborExStart,

  // "In this state the router is describing its entire link state database by
  // sending Database Description packets to the neighbor.  Each Database
  // Description Packet has a DD sequence number, and is explicitly
  // acknowledged.  Only one Database Description Packet is allowed outstanding
  // at any one time.  In this state, Link State Request Packets may also be
  // sent asking for the neighbor's more recent LSAs.  All adjacencies in
  // Exchange state or greater are used by the flooding procedure.  In fact,
  // these adjacencies are fully capable of transmitting and receiving all types
  // of OSPF routing protocol packets."
  NeighborExchange,

  // "In this state, Link State Request packets are sent to the neighbor asking
  // for the more recent LSAs that have been discovered (but not yet received)
  // in the Exchange state."
  NeighborLoading,

  // "In this state, the neighboring routers are fully adjacent.  These
  // adjacencies will now appear in router-LSAs and network-LSAs."
  NeighborFull
};

// RFC 2328 10.  The Neighbor Data Structure
// 主机字节序
struct OspfNeighbor {
  // State
  // "The functional level of the neighbor conversation.  This is described in
  // more detail in Section 10.1."
  enum ospf_neighbor_state state;

  // Master/Slave
  // "When the two neighbors are exchanging databases, they form a master/slave
  // relationship.  The master sends the first Database Description Packet, and
  // is the only part that is allowed to retransmit.  The slave can only respond
  // to the master's Database Description Packets.  The master/slave
  // relationship is negotiated in state ExStart."
  bool is_slave;

  // DD Sequence Number
  // "The DD Sequence number of the Database Description packet that is
  // currently being sent to the neighbor."
  uint32_t dd_sequence_number; // Database Description sequence number

  // Neighbor ID
  // "The OSPF Router ID of the neighboring router.  The Neighbor ID is learned
  // when Hello packets are received from the neighbor, or is configured if this
  // is a virtual adjacency (see Section C.4)."
  uint32_t router_id;

  // Last received Database Description packet
  // "The initialize(I), more (M) and master(MS) bits, Options field, and DD
  // sequence number contained in the last Database Description packet received
  // from the neighbor. Used to determine whether the next Database Description
  // packet received from the neighbor is a duplicate."
  struct {
    uint8_t flags;               // I + M + MS
    uint16_t options;            // Low 16 bits of Options field
    uint32_t dd_sequence_number; // DD sequence number
  } last_received_dd;

  // Link state retransmission list
  // "The list of LSAs that have been flooded but not acknowledged on this
  // adjacency.  These will be retransmitted at intervals until they are
  // acknowledged, or until the adjacency is destroyed."
  std::vector<Lsa> link_state_retransmission_list;

  // Database summary list
  // "The complete list of LSAs that make up the area link-state database, at
  // the moment the neighbor goes into Database Exchange state.  This list is
  // sent to the neighbor in Database Description packets."
  std::vector<Lsa> database_summary_list;

  // Link state request list
  // "The list of LSAs that need to be received from this neighbor in order to
  // synchronize the two neighbors' link-state databases.  This list is created
  // as Database Description packets are received, and is then sent to the
  // neighbor in Link State Request packets.  The list is depleted as
  // appropriate Link State Update packets are received."
  std::vector<struct ospf_lsa_header> link_state_request_list;

  // RFC 5340 4.1.3.  The Neighbor Data Structure
  // Neighbor's Interface ID
  // "The Interface ID that the neighbor advertises in its Hello packets must be
  // recorded in the neighbor structure.  The router will include the neighbor's
  // Interface ID in the router's router-LSA when either a) advertising a
  // point-to-point or point-to-multipoint link to the neighbor or b)
  // advertising a link to a network where the neighbor has become the
  // Designated Router."
  uint32_t interface_id;

  // RFC 5340 4.1.3.  The Neighbor Data Structure
  // Neighbor IP address
  // "The neighbor's IPv6 address contained as the source address in
  //  OSPF for IPv6 packets.  This will be an IPv6 link-local address
  //  for all link types except virtual links."
  in6_addr ip_address;
};

// neighbor states
// 邻居状态，每个接口上允许有多个邻居
std::vector<struct OspfNeighbor> neighbors[N_IFACE_ON_BOARD];

// RFC 2328 Page 121 12.2. The link state database
// 主机字节序
struct LinkStateDB {
  // this router's Router-LSA
  // 本路由器自己的 Router-LSA
  RouterLsa own_router_lsa;
  // this router's Intra-Area-Prefix LSA
  // 本路由器自己的 Intra-Area-Prefix LSA
  IntraAreaPrefixLsa own_intra_area_prefix_lsa;
  // and other routers' Router-LSA/Intra-Area-Prefix-LSA
  // 和其他路由器的 Router-LSA/Intra-Area-Prefix-LSA
  std::vector<Lsa> others_lsas;

  // 导出当前的所有 LSA
  std::vector<Lsa> dumpLSA() const {
    std::vector<Lsa> result;

    // Router-LSA
    Lsa lsa;
    lsa.type = OSPF_ROUTER_LSA;
    lsa.router_lsa = own_router_lsa;
    result.push_back(lsa);

    // Intra-Area-Prefix-LSA
    lsa.type = OSPF_INTRA_AREA_PREFIX_LSA;
    lsa.intra_area_perfix_lsa = own_intra_area_prefix_lsa;
    result.push_back(lsa);

    // Others
    for (auto lsa : others_lsas) {
      result.push_back(lsa);
    }

    return result;
  }

  // 接收到来自其他路由器的 Router-LSA
  // 如果出现了更新，返回 true；否则返回 false
  bool updateLSA(Lsa new_lsa) {
    if (new_lsa.get_advertising_router() == router_id) {
      // 该 LSA 来自本路由器
      return false;
    }

    // RFC 2328 Page 143 13. The Flooding Procedure
    // 先寻找有没有匹配的
    for (auto &lsa : others_lsas) {
      // RFC 2328 Page 146 13.1.  Determining which LSA is newer
      // "An LSA is identified by its LS type, Link State ID and Advertising
      // Router."
      if (lsa.type == new_lsa.type &&
          lsa.get_link_state_id() == new_lsa.get_link_state_id() &&
          lsa.get_advertising_router() == new_lsa.get_advertising_router()) {
        // 匹配到了，判断是否 new_lsa 比 lsa 新

        if (new_lsa.get_ls_sequence_number() > lsa.get_ls_sequence_number()) {
          // 更新 LSA
          lsa = new_lsa;
          printf("LSA updated\n");
          return true;
        }

        // LSA 不变
        return false;
      }
    }

    // 没有匹配的，加入到 LSDB
    others_lsas.push_back(new_lsa);
    printf("Installed new lsa\n");
    return true;
  }

  // 根据 LSDB 计算出新的路由表
  void recalculateRoutingTable() {
    // TODO（20-40 行）
    // 根据 own_router_lsa 和 others_lsas 中的 Router-LSA
    // 构建一个有向图（例如用邻接表）
    // 例如：
    // Router LSA: A 有 neighbor B and C
    // 建立有向边 A 到 B、A 到 C，距离是对应的 RouterLsaEntry.metric 字段

    // TODO（30 行）
    // 实现 Dijkstra 算法
    // 计算从当前路由器（router_id）到其他路由器的最短距离
    // 并记录到每个路由器的最短路径（例如通过 prev 数组记录）

    // TODO（40-60 行）
    // 对于每个除了本路由器以外的路由器，
    // 如果它可达（距离不等于无穷大），找到最短路径
    // 那么最短路径的第一跳就是 nexthop
    // 例如：当前路由器是 A，A 连接到 B，B 连接到 C，C 连接到 D
    // A 到 D 的最短路径是 A-B-C-D
    // 因此 A 到 D 的最短路径上的 nexthop 是 B

    // 枚举所有路由器的所有 Intra-Area-Prefix-LSA
    // 根据它的 Reference Advertising Router，得到它的距离
    // 例如：当前路由器是 A，A 连接到 B，B 连接到 C，C 连接到 N 网络
    // C 会给 N 网络产生一个 Intra-Area-Prefix-LSA
    // 于是 A 到 N 网络的距离，就是 A 到 C 的距离（Dijkstra 算法求得），加上 C
    // 到 N 的距离（IntraAreaPrefixLSAEntry.metric 字段）
    // 对于每个路由前缀（N 网络），找到它距离最近的一个路径（A-B-C-N）和对应的
    // nexthop（B）和 if_index（A 连往 B 的 if_index，可以在邻居表中找到），
    // 插入路由表（使用 lookup 小作业的 update 函数）
    //
    // 注意同一个网络可能有多个 Intra-Area-Prefix-LSA，例如：
    // A 连接到 B，B 连接到 C：A-B-C
    // B 会给 B 到 C 的网络 N1 产生 Intra-Area-Prefix-LSA。
    // C 也会给 C 到 B 的网络 N2 产生 Intra-Area-Prefix-LSA：
    //        N1  N2
    // A -- B ------ C
    // 此时距离更近的路径是 A-B-N1，而不是 A-B-C-N2。
  }
};

// link state database
// 链路状态数据库
struct LinkStateDB lsdb;

// 作为 slave 端，发送 OSPF Database Description 逻辑
void ospf_dd_send_slave(int if_index, struct ip6_hdr *ip6,
                        struct ospf_database_description *ospf,
                        OspfNeighbor &neighbor, ether_addr src_mac) {
  // 发送 Database Description Packet
  // RFC 2328 Page 102
  // "The slave must send a Database Description Packet in
  // reply."
  ether_addr mac;
  HAL_GetInterfaceMacAddress(if_index, &mac);

  // IPv6 header
  ip6_hdr *reply_ip6 = (ip6_hdr *)&output[0];
  // flow label
  reply_ip6->ip6_flow = 0;
  // version
  reply_ip6->ip6_vfc = 6 << 4;
  // next header
  reply_ip6->ip6_nxt = 89;
  // hop limit
  reply_ip6->ip6_hlim = 1;
  // src ip
  reply_ip6->ip6_src = eui64(mac);
  // dst ip
  reply_ip6->ip6_dst = ip6->ip6_src;

  // 发送 Database Summary List 中 LSA 的 LSA Header
  ospf_database_description *reply_ospf =
      (ospf_database_description *)&output[sizeof(ip6_hdr)];
  uint16_t ospf_len = sizeof(struct ospf_database_description);
  std::vector<Lsa> &database_summary_list = neighbor.database_summary_list;
  int j;
  for (j = 0; j < OSPF_DD_MAX_LSA_HEADER && j < database_summary_list.size();
       j++) {
    Lsa &lsa = database_summary_list[j];
    ospf_fill_lsa_header(lsa, &reply_ospf->lsa_header[j]);
    ospf_len += sizeof(struct ospf_lsa_header);
  }
  // 去掉已经发送的部分
  database_summary_list.erase(database_summary_list.begin(),
                              database_summary_list.begin() + j);

  // OSPFv3
  reply_ospf->header.version = 3;
  // Database Description
  reply_ospf->header.type = 2;
  reply_ospf->header.length = htons(ospf_len);
  reply_ospf->header.router_id = htonl(router_id);
  reply_ospf->header.area_id = 0;
  reply_ospf->header.instance = 0;
  reply_ospf->header.zero = 0;

  reply_ospf->zero = 0;
  // V6(0x1) | E(0x2) | R(0x10)
  reply_ospf->options = htons(0x1 | 0x2 | 0x10);
  // MTU = 1500
  reply_ospf->interface_mtu = htons(1500);
  // I=MS=0
  uint16_t flags = 0;
  // 如果 database_summary_list 还有元素，设置 M=1
  if (!database_summary_list.empty()) {
    flags |= 0x2; // M=1
  }

  // RFC 2328 Page 102
  // "If the received packet has the more bit (M) set to 0, and the packet to be
  // sent by the slave will also have the M-bit set to 0, the neighbor event
  // ExchangeDone is generated.  Note that the slave always generates this event
  // before the master."
  if ((ntohs(ospf->flags) & 0x2) == 0x0) {
    flags &= ~0x2;
  }

  reply_ospf->flags = htons(flags);

  // ack master's sequence number
  // RFC 2328 Page 102
  // "Sets the DD sequence number in the neighbor data structure
  // to the DD sequence number appearing in the received
  // packet."
  neighbor.dd_sequence_number = ntohl(ospf->sequence_number);
  reply_ospf->sequence_number = ospf->sequence_number;

  uint16_t ip_len = ospf_len + sizeof(ip6_hdr);
  reply_ip6->ip6_plen = htons(ospf_len);

  // compute checksum
  validateAndFillChecksum(output, ip_len);
  HAL_SendIPPacket(if_index, output, ip_len, src_mac);
}

// 作为 master 端，发送 OSPF Database Description 逻辑
void ospf_dd_send_master(int if_index, struct ip6_hdr *ip6,
                         struct ospf_database_description *ospf,
                         OspfNeighbor &neighbor, ether_addr src_mac) {
  // 发送 Database Description Packet
  ether_addr mac;
  HAL_GetInterfaceMacAddress(if_index, &mac);

  // IPv6 header
  ip6_hdr *reply_ip6 = (ip6_hdr *)&output[0];
  // flow label
  reply_ip6->ip6_flow = 0;
  // version
  reply_ip6->ip6_vfc = 6 << 4;
  // next header
  reply_ip6->ip6_nxt = 89;
  // hop limit
  reply_ip6->ip6_hlim = 1;
  // src ip
  reply_ip6->ip6_src = eui64(mac);
  // dst ip
  reply_ip6->ip6_dst = ip6->ip6_src;

  // 发送 Database Summary List 中 LSA 的 LSA Header
  ospf_database_description *reply_ospf =
      (ospf_database_description *)&output[sizeof(ip6_hdr)];
  uint16_t ospf_len = sizeof(struct ospf_database_description);
  std::vector<Lsa> &database_summary_list = neighbor.database_summary_list;
  int j;
  for (j = 0; j < OSPF_DD_MAX_LSA_HEADER && j < database_summary_list.size();
       j++) {
    Lsa &lsa = database_summary_list[j];
    ospf_fill_lsa_header(lsa, &reply_ospf->lsa_header[j]);
    ospf_len += sizeof(struct ospf_lsa_header);
  }
  // 去掉已经发送的部分
  database_summary_list.erase(database_summary_list.begin(),
                              database_summary_list.begin() + j);

  // OSPFv3
  reply_ospf->header.version = 3;
  // Database Description
  reply_ospf->header.type = 2;
  reply_ospf->header.length = htons(ospf_len);
  reply_ospf->header.router_id = htonl(router_id);
  reply_ospf->header.area_id = 0;
  reply_ospf->header.instance = 0;
  reply_ospf->header.zero = 0;

  reply_ospf->zero = 0;
  // V6(0x1) | E(0x2) | R(0x10)
  reply_ospf->options = htons(0x1 | 0x2 | 0x10);
  // MTU = 1500
  reply_ospf->interface_mtu = htons(1500);
  // I=0, M=MS=1
  uint16_t flags = 0x1 | 0x2;

  // RFC 2328 Page 102
  // "If the router has already sent its entire sequence of Database Description
  // Packets, and the just accepted packet has the more bit (M) set to 0"
  if ((ntohs(ospf->flags) & 0x2) == 0x0 && database_summary_list.empty()) {
    flags &= ~0x2;
  }

  reply_ospf->flags = htons(flags);

  // increase sequence number and send
  // RFC 2328 Page 102
  // "Increments the DD sequence number in the neighbor data
  // structure."
  neighbor.dd_sequence_number += 1;
  reply_ospf->sequence_number = htonl(neighbor.dd_sequence_number);

  uint16_t ip_len = ospf_len + sizeof(ip6_hdr);
  reply_ip6->ip6_plen = htons(ospf_len);

  // compute checksum
  validateAndFillChecksum(output, ip_len);
  HAL_SendIPPacket(if_index, output, ip_len, src_mac);
}

void ospf_send_ls_update(int if_index, OspfNeighbor &neighbor, Lsa &lsa) {
  // 向指定邻居路由器发送 OSPF LS Update Packet
  ether_addr mac;
  HAL_GetInterfaceMacAddress(if_index, &mac);

  // IPv6 header
  ip6_hdr *reply_ip6 = (ip6_hdr *)&output[0];
  // flow label
  reply_ip6->ip6_flow = 0;
  // version
  reply_ip6->ip6_vfc = 6 << 4;
  // next header
  reply_ip6->ip6_nxt = 89;
  // hop limit
  reply_ip6->ip6_hlim = 1;
  // src ip
  reply_ip6->ip6_src = eui64(mac);
  // dst ip
  reply_ip6->ip6_dst = neighbor.ip_address;

  // Fill LSA content
  ospf_lsa_header *reply_lsa =
      (ospf_lsa_header *)&output[sizeof(ip6_hdr) + sizeof(ospf_header) + 4];
  uint16_t lsa_len = ospf_fill_lsa(lsa, reply_lsa);
  uint16_t ospf_len = sizeof(ospf_header) + sizeof(uint32_t) + lsa_len;
  ospf_ls_update *reply_ospf = (ospf_ls_update *)&output[sizeof(ip6_hdr)];
  // OSPFv3
  reply_ospf->header.version = 3;
  // LS Update
  reply_ospf->header.type = 4;
  reply_ospf->header.length = htons(ospf_len);
  reply_ospf->header.router_id = htonl(router_id);
  reply_ospf->header.area_id = 0;
  reply_ospf->header.instance = 0;
  reply_ospf->header.zero = 0;

  // only one LSA
  reply_ospf->lsas = htonl(1);

  uint16_t ip_len = ospf_len + sizeof(ip6_hdr);
  reply_ip6->ip6_plen = htons(ospf_len);

  // compute checksum
  validateAndFillChecksum(output, ip_len);
  ether_addr dest_mac;
  if (HAL_GetNeighborMacAddress(if_index, neighbor.ip_address, &dest_mac) ==
      0) {
    HAL_SendIPPacket(if_index, output, ip_len, dest_mac);
  }
}

void ospf_flood_lsa(Lsa &lsa) {
  // 向每个邻居路由器发送 OSPF LS Update Packet
  printf("Flood LSA to neighbors\n");
  for (int if_index = 0; if_index < N_IFACE_ON_BOARD; if_index++) {
    for (auto &neighbor : neighbors[if_index]) {
      // 如果 Link state retransmission list 已经有该 LSA，则删除已有的
      for (int k = 0; k < neighbor.link_state_retransmission_list.size(); k++) {
        if (neighbor.link_state_retransmission_list[k].type == lsa.type &&
            neighbor.link_state_retransmission_list[k].get_link_state_id() ==
                lsa.get_link_state_id() &&
            neighbor.link_state_retransmission_list[k]
                    .get_advertising_router() == lsa.get_advertising_router()) {
          printf("Remove old lsa from retransmission list\n");
          neighbor.link_state_retransmission_list.erase(
              neighbor.link_state_retransmission_list.begin() + k);
          k--;
        }
      }

      // 记录到 Link state retransmission list 中，用于重传 LSA
      neighbor.link_state_retransmission_list.push_back(lsa);

      // 发送 LSA
      ospf_send_ls_update(if_index, neighbor, lsa);
    }
  }
}

void ospf_finish_sync(int if_index, OspfNeighbor *neighbor) {
  // 和邻居路由器完成了 LSDB 同步，进入 Full 状态
  // 更新 Router LSA，添加 neighbor
  // RFC 2328 Page 83
  // "When this synchronization is finished, the neighbor
  // is in state Full and we say that the two routers are
  // fully adjacent.  At this point the adjacency is listed in
  // LSAs."
  RouterLsaEntry entry;
  // Point-to-point connection to another router
  entry.type = 1;
  entry.metric = 1;
  entry.interface_id = if_index;
  entry.neighbor_router_id = neighbor->router_id;
  entry.neighbor_interface_id = neighbor->interface_id;
  lsdb.own_router_lsa.entries.push_back(entry);
  lsdb.own_router_lsa.ls_sequence_number += 1;
  // 更新路由表
  lsdb.recalculateRoutingTable();

  // 广播 LSA 更新
  Lsa lsa;
  lsa.type = OSPF_ROUTER_LSA;
  lsa.router_lsa = lsdb.own_router_lsa;
  ospf_flood_lsa(lsa);
}

int main(int argc, char *argv[]) {
  // 初始化 HAL
  int res = HAL_Init(1, addrs);
  if (res < 0) {
    return res;
  }

  // 插入直连路由
  // 例如 R2：
  // fd00::3:0/112 if 0
  // fd00::4:0/112 if 1
  // fd00::8:0/112 if 2
  // fd00::9:0/112 if 3
  for (uint32_t i = 0; i < N_IFACE_ON_BOARD; i++) {
    in6_addr mask = len_to_mask(112);
    RoutingTableEntry entry = {
        .addr = addrs[i] & mask,
        .len = 112,
        .if_index = i,
        .nexthop = in6_addr{0}, // 全 0 表示直连路由
    };
    update(true, entry);
  }

#ifdef ROUTER_INTERCONNECT
  // 互联测试
  // 添加路由：
  // fd00::1:0/112 via fd00::3:1 if 0
  RoutingTableEntry entry = {
      .addr = {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
               0x00, 0x00, 0x01, 0x00, 0x00},
      .len = 112,
      .if_index = 0,
      .nexthop = {0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                  0x00, 0x00, 0x00, 0x03, 0x00, 0x01},
  };
  update(true, entry);
#endif

  // initialize the router's own Router-LSA
  lsdb.own_router_lsa.ls_age = 1;
  lsdb.own_router_lsa.link_state_id = 0;
  lsdb.own_router_lsa.advertising_router = router_id;
  lsdb.own_router_lsa.ls_sequence_number = 0x80000001;
  lsdb.own_router_lsa.flags = 0;
  lsdb.own_router_lsa.zero = 0;
  // V6(0x1) | E(0x2) | R(0x10)
  lsdb.own_router_lsa.options = 0x13;

  // lsdb.own_router_lsa.entries 会在和邻居路由器完成数据同步后更新
  // RFC 2328 Page 54
  // "When the Database Description Process has completed and all Link State
  // Requests have been satisfied, the databases are deemed synchronized and the
  // routers are marked fully adjacent.  At this time the adjacency is fully
  // functional and is advertised in the two routers' router-LSAs."

  // initialize the router's Intra-Area-Prefix-LSA
  lsdb.own_intra_area_prefix_lsa.ls_age = 1;
  lsdb.own_intra_area_prefix_lsa.link_state_id = 0;
  lsdb.own_intra_area_prefix_lsa.advertising_router = router_id;
  lsdb.own_intra_area_prefix_lsa.ls_sequence_number = 0x80000001;
  lsdb.own_intra_area_prefix_lsa.referenced_ls_type = OSPF_ROUTER_LSA;
  lsdb.own_intra_area_prefix_lsa.referenced_link_state_id = 0;
  lsdb.own_intra_area_prefix_lsa.referenced_advertising_router = router_id;
  for (uint32_t i = 0; i < N_IFACE_ON_BOARD; i++) {
    IntraAreaPrefixLsaEntry entry;
    in6_addr mask = len_to_mask(112);
    entry.prefix_length = 112;
    entry.prefix_options = 0;
    entry.metric = 1;
    entry.address_prefix = addrs[i] & mask;
    lsdb.own_intra_area_prefix_lsa.entries.push_back(entry);
  }

  uint64_t last_time = 0;
  while (1) {
    uint64_t time = HAL_GetTicks();
    // RFC 要求每 HelloInterval 秒发送一次 OSPF Hello Packet
    if (time > last_time + 5 * 1000) {
      // 提示：你可以打印完整的路由表到 stdout/stderr 来帮助调试。
      printf("5s Timer\n");

      for (int i = 0; i < N_IFACE_ON_BOARD; i++) {
        ether_addr mac;
        HAL_GetInterfaceMacAddress(i, &mac);

        // IPv6 header
        ip6_hdr *reply_ip6 = (ip6_hdr *)&output[0];
        // flow label
        reply_ip6->ip6_flow = 0;
        // version
        reply_ip6->ip6_vfc = 6 << 4;
        // next header
        reply_ip6->ip6_nxt = 89;
        // hop limit
        reply_ip6->ip6_hlim = 1;
        // src ip
        reply_ip6->ip6_src = eui64(mac);
        // dst ip
        reply_ip6->ip6_dst = inet6_pton("ff02::5");

        uint16_t ospf_len =
            sizeof(struct ospf_hello) + sizeof(uint32_t) * neighbors[i].size();
        ospf_hello *ospf = (ospf_hello *)&output[sizeof(ip6_hdr)];

        // TODO（10 行）
        // 发送 OSPF Hello 消息
        // 头部部分：
        // 设置 OSPF 版本为 3，Type 为 1（Hello）
        // Length 为 ospf_len（注意端序）
        // Router ID 为本路由器的 router_id（注意端序）
        // Area ID、Instance 和 Zero 等于 0
        // OSPF Hello 部分：
        // Interface ID 等于 i（注意端序）
        // Router Priority 等于 1
        // Options 等于 0x13 = V6(0x1) | E(0x2) | R(0x10)（注意端序）
        // Hello Interval 等于 5s（注意端序）
        // Router Dead Interval 等于 20s（注意端序）
        // Designated Router ID 和 Backup Designated Router ID 都等于 0.0.0.0

        // Neighbor ID
        // RFC 2823 Page 78
        // "In order to ensure two-way communication between adjacent routers,
        // the Hello packet contains the list of all routers on the network from
        // which Hello Packets have been seen recently."
        for (int j = 0; j < neighbors[i].size(); j++) {
          // TODO（1 行）
          // 对于 neighbors[i] 里的每个邻居 neighbors[i][j] 的 Router ID
          // 把它填入 OSPF Hello 的 neighbor_id 中
        }

        uint16_t ip_len = ospf_len + sizeof(ip6_hdr);
        reply_ip6->ip6_plen = htons(ospf_len);

        // compute checksum
        // TODO（5 行）
        // 修改 checksum 小作业，加入对 OSPF 的支持
        // OSPF 和 ICMPv6 类似，区别是 OSPF 的 ip6_nxt 是 89
        validateAndFillChecksum(output, ip_len);
        HAL_SendIPPacket(i, output, ip_len,
                         {0x33, 0x33, 0x00, 0x00, 0x00, 0x05});
      }

      // 如果 link state retransmission list 非空，则重新发送 LS Update
      for (int i = 0; i < N_IFACE_ON_BOARD; i++) {
        for (auto &neighbor : neighbors[i]) {
          for (auto &lsa : neighbor.link_state_retransmission_list) {
            printf("Retransmit LS Update\n");
            ospf_send_ls_update(i, neighbor, lsa);
          }
        }
      }

      last_time = time;
    }

    int mask = (1 << N_IFACE_ON_BOARD) - 1;
    ether_addr src_mac;
    ether_addr dst_mac;
    int if_index;
    res = HAL_ReceiveIPPacket(mask, packet, sizeof(packet), &src_mac, &dst_mac,
                              1000, &if_index);
    if (res == HAL_ERR_EOF) {
      break;
    } else if (res < 0) {
      return res;
    } else if (res == 0) {
      // Timeout
      continue;
    } else if (res > sizeof(packet)) {
      // packet is truncated, ignore it
      continue;
    }

    // 检查 IPv6 头部长度
    ip6_hdr *ip6 = (ip6_hdr *)packet;
    if (res < sizeof(ip6_hdr)) {
      printf("Received invalid ipv6 packet (%d < %d)\n", res, sizeof(ip6_hdr));
      continue;
    }
    uint16_t plen = ntohs(ip6->ip6_plen);
    if (res < plen + sizeof(ip6_hdr)) {
      printf("Received invalid ipv6 packet (%d < %d + %d)\n", res, plen,
             sizeof(ip6_hdr));
      continue;
    }

    // 检查 IPv6 头部目的地址是否为我自己
    bool dst_is_me = false;
    for (int i = 0; i < N_IFACE_ON_BOARD; i++) {
      if (memcmp(&ip6->ip6_dst, &addrs[i], sizeof(in6_addr)) == 0) {
        dst_is_me = true;
        break;
      }
    }

    // TODO（1 行）
    // 修改这个检查，当目的地址为 OSPF 的组播目的地址（ff02::5）时也设置
    // dst_is_me 为 true。
    if (false) {
      dst_is_me = true;
    }

    // 判断是否是 Link Local 地址
    ether_addr mac;
    HAL_GetInterfaceMacAddress(if_index, &mac);
    if (ip6->ip6_dst == eui64(mac)) {
      dst_is_me = true;
    }

    if (dst_is_me) {
      // 目的地址是我，按照类型进行处理

      // 如果是 OSPFv3 packet，检查 checksum 是否正确
      if (ip6->ip6_nxt == 89) {
        if (!validateAndFillChecksum(packet, res)) {
          printf("Received packet with bad checksum\n");
          continue;
        }

        ospf_header *ospf_hdr = (ospf_header *)&packet[sizeof(ip6_hdr)];

        // 转换 Router Id 为字符串，方便后续打印
        char neighbor_router_id_buffer[64];
        inet_ntop(AF_INET, &ospf_hdr->router_id, neighbor_router_id_buffer,
                  sizeof(neighbor_router_id_buffer));

        // 检查是否是已知的 Neighbor
        uint32_t neighbor_router_id = ntohl(ospf_hdr->router_id);
        bool neighbor_found = false;
        OspfNeighbor *neighbor = nullptr;
        for (int i = 0; i < neighbors[if_index].size(); i++) {
          if (neighbors[if_index][i].router_id == neighbor_router_id) {
            neighbor_found = true;
            neighbor = &neighbors[if_index][i];
            break;
          }
        }

        if (ospf_hdr->type == 1 && ospf_hdr->version == 3) {
          // 如果是 OSPF Hello Packet
          // RFC 2328 10.5.  Receiving Hello Packets
          // https://www.rfc-editor.org/rfc/rfc2328.html#page-96
          // RFC 5340 4.2.2.1.  Receiving Hello Packets
          // https://www.rfc-editor.org/rfc/rfc5340.html#section-4.2.2.1

          ospf_hello *hello = (ospf_hello *)&packet[sizeof(ip6_hdr)];
          printf("Received OSPF Hello Packet from %s\n",
                 neighbor_router_id_buffer);

          // 如果是已知的 Neighbor，且对方的 Neighbor 出现了自己的 Router
          // Id，进入 ExStart 状态
          //
          // RFC2328 Page 98:
          // "Then the list of neighbors contained in the Hello Packet is
          // examined.  If the router itself appears in this list, the
          // neighbor state machine should be executed with the event 2-
          // WayReceived."
          //
          // RFC2328 Page 90-91:
          // "State(s):  Init"
          // "Event:  2-WayReceived"
          // "Otherwise (an adjacency should be established) the
          // neighbor state transitions to ExStart."
          if (neighbor_found && neighbor->state == NeighborInit) {
            int num_neighbors =
                (ntohl(ospf_hdr->length) - sizeof(struct ospf_hello)) /
                sizeof(uint32_t);
            for (int i = 0; i < num_neighbors; i++) {
              // TODO（1 行）
              // 修改这个检查，完成 Hello 的协商判断 当发现对方发送的 OSPF Hello
              // 的 neighbor 列表出现了本路由器的 router id，进入分支
              if (false) {
                printf("Neighbor %s enters ExStart state\n",
                       neighbor_router_id_buffer);
                neighbor->state = NeighborExStart;

                // RFC2328 Page 91
                // "... starts sending Database Description Packets, ...
                // This Database Description Packet should be otherwise
                // empty."
                // 发送 Database Description
                ether_addr mac;
                HAL_GetInterfaceMacAddress(if_index, &mac);

                // IPv6 header
                ip6_hdr *reply_ip6 = (ip6_hdr *)&output[0];
                // flow label
                reply_ip6->ip6_flow = 0;
                // version
                reply_ip6->ip6_vfc = 6 << 4;
                // next header
                reply_ip6->ip6_nxt = 89;
                // hop limit
                reply_ip6->ip6_hlim = 1;
                // src ip
                reply_ip6->ip6_src = eui64(mac);
                // dst ip
                reply_ip6->ip6_dst = ip6->ip6_src;

                uint16_t ospf_len = sizeof(struct ospf_database_description);
                ospf_database_description *reply_ospf =
                    (ospf_database_description *)&output[sizeof(ip6_hdr)];
                // OSPFv3
                reply_ospf->header.version = 3;
                // Database Description
                reply_ospf->header.type = 2;
                reply_ospf->header.length = htons(ospf_len);
                reply_ospf->header.router_id = htonl(router_id);
                reply_ospf->header.area_id = 0;
                reply_ospf->header.instance = 0;
                reply_ospf->header.zero = 0;

                reply_ospf->zero = 0;
                // V6(0x1) | E(0x2) | R(0x10)
                reply_ospf->options = htons(0x1 | 0x2 | 0x10);
                // MTU = 1500
                reply_ospf->interface_mtu = htons(1500);

                // RFC2328 Page 91
                // "It then declares itself master (sets the master/slave
                // bit to master), and starts sending Database Description
                // Packets, with the initialize (I), more (M) and master
                // (MS) bits set."
                // I(0x4) | M(0x2) | MS(0x1)
                reply_ospf->flags = htons(0x4 | 0x2 | 0x1);

                // set initial sequence number
                // RFC2328 Page 91
                // "... the DD sequence number should be assigned some
                // unique value (like the time of day clock)."
                uint32_t seq_num = rand();
                neighbor->dd_sequence_number = seq_num;
                reply_ospf->sequence_number = htonl(seq_num);

                uint16_t ip_len = ospf_len + sizeof(ip6_hdr);
                reply_ip6->ip6_plen = htons(ospf_len);

                // compute checksum
                validateAndFillChecksum(output, ip_len);
                HAL_SendIPPacket(if_index, output, ip_len, src_mac);
                break;
              }
            }
          }

          // 如果是新的 Neighbor，初始化 Neighbor State 并开始握手
          if (!neighbor_found) {
            OspfNeighbor new_neighbor;
            new_neighbor.state = NeighborInit;
            new_neighbor.router_id = neighbor_router_id;
            new_neighbor.interface_id = ntohl(hello->interface_id);
            new_neighbor.ip_address = ip6->ip6_src;
            neighbors[if_index].push_back(new_neighbor);
            printf("Discovered new neighbor %s\n", neighbor_router_id_buffer);

            // 为了简化实现，立即触发一次 Hello 发送
            last_time = 0;
          }
        } else if (ospf_hdr->type == 2 && ospf_hdr->version == 3) {
          // 如果是 OSPF Database Description Packet
          // RFC 2328 10.6.  Receiving Database Description Packets
          printf("Received OSPF Database Description Packet from %s\n",
                 neighbor_router_id_buffer);
          ospf_database_description *ospf =
              (ospf_database_description *)&packet[sizeof(ip6_hdr)];

          if (neighbor_found && neighbor->state == NeighborExStart) {
            // RFC2328 Page 100
            // "If the received packet matches one of the following cases,
            // ..."
            //
            // "The initialize(I), more (M) and master(MS) bits are set,
            // the contents of the packet are empty, and the neighbor's
            // Router ID is larger than the router's own.  In this case
            // the router is now Slave."
            if ( // I | M | MS
                (ntohs(ospf->flags) & 0x7) == 0x7 &&
                // contents of the packet are empty
                ntohs(ospf->header.length) == 28 &&
                // neighbor's Router ID is larger than the
                // router's own
                neighbor_router_id > router_id) {
              printf("Become slave in Exchange with %s\n",
                     neighbor_router_id_buffer);
              neighbor->is_slave = true;

              // RFC2328 Page 100
              // "... then the neighbor state machine should be executed
              // with the event NegotiationDone (causing the state to
              // transition to Exchange)"
              printf("Neighbor %s enters Exchange state\n",
                     neighbor_router_id_buffer);
              neighbor->state = NeighborExchange;

              // RFC2328 Page 81
              // "The complete list of LSAs that make up the area link-state
              // database, at the moment the neighbor goes into Database
              // Exchange state"
              neighbor->database_summary_list = lsdb.dumpLSA();

              // RFC2328 Page 100
              // "... and set the neighbor data structure's DD sequence
              // number to that specified by the master."
              neighbor->dd_sequence_number = ntohl(ospf->sequence_number);

              // 发送 Database Description Packet
              // RFC 2328 Page 102
              // "The slave must send a Database Description Packet in
              // reply."
              ospf_dd_send_slave(if_index, ip6, ospf, *neighbor, src_mac);
            } else if (
                // I=MS=0
                (ntohs(ospf->flags) & 0x5) == 0x0 &&
                // the packet's DD sequence number equals the neighbor data
                // structure's DD sequence number
                ntohl(ospf->sequence_number) == neighbor->dd_sequence_number &&
                // the neighbor's Router ID is smaller than the router's own
                neighbor_router_id < router_id) {
              // RFC 2328 Page 100
              // "The initialize(I) and master(MS) bits are off, the
              // packet's DD sequence number equals the neighbor data
              // structure's DD sequence number (indicating
              // acknowledgment) and the neighbor's Router ID is smaller
              // than the router's own.  In this case the router is
              // Master."

              printf("Become master in Exchange with %s\n",
                     neighbor_router_id_buffer);
              neighbor->is_slave = false;

              // RFC 2328 Page 100
              // "... then the neighbor state machine should be executed
              // with the event NegotiationDone (causing the state to
              // transition to Exchange)"
              printf("Neighbor %s enters Exchange state\n",
                     neighbor_router_id_buffer);
              neighbor->state = NeighborExchange;

              // RFC 2328 Page 81
              // "The complete list of LSAs that make up the area link-state
              // database, at the moment the neighbor goes into Database
              // Exchange state"
              neighbor->database_summary_list = lsdb.dumpLSA();

              // 把对方发送的 LSA header 保存到 link_state_request_list 中
              // 为了简化，直接请求所有的 LSA，而不是只请求自己没有的 LSA
              // RFC 2823 Page 82
              // "This list is created as Database Description packets are
              // received, and is then sent to the neighbor in Link State
              // Request packets."
              int num_lsa_headers = (ntohs(ospf->header.length) -
                                     sizeof(struct ospf_database_description)) /
                                    sizeof(struct ospf_lsa_header);
              for (int j = 0; j < num_lsa_headers; j++) {
                neighbor->link_state_request_list.push_back(
                    ospf->lsa_header[j]);
              }

              // 发送 Database Description
              ospf_dd_send_master(if_index, ip6, ospf, *neighbor, src_mac);
            }
          } else if (neighbor_found && neighbor->state == NeighborExchange) {
            // 把对方发送的 LSA header 保存到 link_state_request_list 中
            // 为了简化，直接请求所有的 LSA，而不是只请求自己没有的 LSA
            // RFC 2823 Page 82
            // "This list is created as Database Description packets are
            // received, and is then sent to the neighbor in Link State
            // Request packets."
            int num_lsa_headers = (ntohs(ospf->header.length) -
                                   sizeof(struct ospf_database_description)) /
                                  sizeof(struct ospf_lsa_header);
            for (int j = 0; j < num_lsa_headers; j++) {
              neighbor->link_state_request_list.push_back(ospf->lsa_header[j]);
            }

            bool event_exchange_done = false;
            if (neighbor->is_slave) {
              // 发送 Database Description Packet
              // RFC 2328 Page 102
              // "The slave must send a Database Description Packet in
              // reply."
              ospf_dd_send_slave(if_index, ip6, ospf, *neighbor, src_mac);

              // RFC 2328 Page 102
              // "If the received packet has the more bit (M) set to 0, ...
              // the neighbor event ExchangeDone is generated.
              if ((ntohs(ospf->flags) & 0x2) == 0x0) {
                event_exchange_done = true;
              }
            } else {
              // RFC 2328 Page 102
              // "If the router has already sent its entire
              // sequence of Database Description Packets, and the just
              // accepted packet has the more bit (M) set to 0, the neighbor
              // event ExchangeDone is generated."
              if ((ntohs(ospf->flags) & 0x2) == 0x0 &&
                  neighbor->database_summary_list.empty()) {
                event_exchange_done = true;
              } else {
                // 发送 Database Description Packet
                ospf_dd_send_master(if_index, ip6, ospf, *neighbor, src_mac);
              }
            }

            if (event_exchange_done) {
              printf("Finish database exchange with %s\n",
                     neighbor_router_id_buffer);

              // RFC 2328 Page 92
              // "State(s):  Exchange"
              // "Event:  ExchangeDone"
              // "Action:  If the neighbor Link state request list is
              // empty, the new neighbor state is Full.  No other action
              // is required.  This is an adjacency's final state.
              // Otherwise, the new neighbor state is Loading.  Start (or
              // continue) sending Link State Request packets to the
              // neighbor (see Section 10.9).  These are requests for the
              // neighbor's more recent LSAs (which were discovered but
              // not yet received in the Exchange state).  These LSAs are
              // listed in the Link state request list associated with the
              // neighbor."
              if (neighbor->link_state_request_list.empty()) {
                // 和邻居路由器完成了 LSDB 同步，进入 Full 状态
                printf("Neighbor %s enters Full state\n",
                       neighbor_router_id_buffer);
                neighbor->state = NeighborFull;

                // 更新 Router LSA，添加 neighbor
                ospf_finish_sync(if_index, neighbor);
              } else {
                printf("Neighbor %s enters Loading state\n",
                       neighbor_router_id_buffer);
                neighbor->state = NeighborLoading;

                // 对于 link_state_request_list 中的 LSA，发送 LS Request
                auto &link_state_request_list =
                    neighbor->link_state_request_list;
                for (int from = 0; from < link_state_request_list.size();
                     from += OSPF_LS_REQUEST_MAX_LSA_REQUEST) {
                  // [from, to)
                  int to = from + OSPF_LS_REQUEST_MAX_LSA_REQUEST;
                  if (to > link_state_request_list.size()) {
                    to = link_state_request_list.size();
                  }

                  printf("Send OSPF LS Request to %s\n",
                         neighbor_router_id_buffer);

                  // 发送 OSPF LS Request Packet
                  ether_addr mac;
                  HAL_GetInterfaceMacAddress(if_index, &mac);

                  // IPv6 header
                  ip6_hdr *reply_ip6 = (ip6_hdr *)&output[0];
                  // flow label
                  reply_ip6->ip6_flow = 0;
                  // version
                  reply_ip6->ip6_vfc = 6 << 4;
                  // next header
                  reply_ip6->ip6_nxt = 89;
                  // hop limit
                  reply_ip6->ip6_hlim = 1;
                  // src ip
                  reply_ip6->ip6_src = eui64(mac);
                  // dst ip
                  reply_ip6->ip6_dst = ip6->ip6_src;

                  uint16_t ospf_len = sizeof(ospf_header) +
                                      sizeof(ospf_lsa_request) * (to - from);
                  ospf_ls_request *reply_ospf =
                      (ospf_ls_request *)&output[sizeof(ip6_hdr)];
                  // OSPFv3
                  reply_ospf->header.version = 3;
                  // LS Request
                  reply_ospf->header.type = 3;
                  reply_ospf->header.length = htons(ospf_len);
                  reply_ospf->header.router_id = htonl(router_id);
                  reply_ospf->header.area_id = 0;
                  reply_ospf->header.instance = 0;
                  reply_ospf->header.zero = 0;

                  // Fill LSA requests
                  for (int j = from; j < to; j++) {
                    reply_ospf->lsa_request[j - from].zero = 0;
                    reply_ospf->lsa_request[j - from].ls_type =
                        link_state_request_list[j].ls_type;
                    reply_ospf->lsa_request[j - from].link_state_id =
                        link_state_request_list[j].link_state_id;
                    reply_ospf->lsa_request[j - from].advertising_router =
                        link_state_request_list[j].advertising_router;
                  }

                  uint16_t ip_len = ospf_len + sizeof(ip6_hdr);
                  reply_ip6->ip6_plen = htons(ospf_len);

                  // compute checksum
                  validateAndFillChecksum(output, ip_len);
                  HAL_SendIPPacket(if_index, output, ip_len, src_mac);
                }
              }
            }
          }
        } else if (ospf_hdr->type == 3 && ospf_hdr->version == 3) {
          // 如果是 OSPF LS Request Packet
          printf("Received OSPF LS Request from %s\n",
                 neighbor_router_id_buffer);
          ospf_ls_request *ospf = (ospf_ls_request *)&packet[sizeof(ip6_hdr)];

          // 查询到对应的 LSA 并回复
          int lsa_entries = (ntohs(ospf->header.length) - sizeof(ospf_header)) /
                            sizeof(ospf_lsa_request);
          std::vector<Lsa> current_lsa = lsdb.dumpLSA();
          for (int j = 0; j < lsa_entries; j++) {
            ospf_lsa_request request = ospf->lsa_request[j];

            // 寻找匹配的 LSA
            for (auto lsa : current_lsa) {
              if (lsa.type == ntohs(request.ls_type) &&
                  lsa.get_link_state_id() == ntohl(request.link_state_id) &&
                  lsa.get_advertising_router() ==
                      ntohl(request.advertising_router)) {
                // 找到了匹配
                printf("Send OSPF LS Update to %s\n",
                       neighbor_router_id_buffer);

                // 回复 OSPF LS Update Packet
                // 为了简化，一次只回复一个 LSA
                ether_addr mac;
                HAL_GetInterfaceMacAddress(if_index, &mac);

                // IPv6 header
                ip6_hdr *reply_ip6 = (ip6_hdr *)&output[0];
                // flow label
                reply_ip6->ip6_flow = 0;
                // version
                reply_ip6->ip6_vfc = 6 << 4;
                // next header
                reply_ip6->ip6_nxt = 89;
                // hop limit
                reply_ip6->ip6_hlim = 1;
                // src ip
                reply_ip6->ip6_src = eui64(mac);
                // dst ip
                reply_ip6->ip6_dst = ip6->ip6_src;

                // Fill LSA content
                ospf_lsa_header *reply_lsa =
                    (ospf_lsa_header *)&output[sizeof(ip6_hdr) +
                                               sizeof(ospf_header) +
                                               sizeof(uint32_t)];
                uint16_t lsa_len = ospf_fill_lsa(lsa, reply_lsa);
                uint16_t ospf_len =
                    sizeof(ospf_header) + sizeof(uint32_t) + lsa_len;
                ospf_ls_update *reply_ospf =
                    (ospf_ls_update *)&output[sizeof(ip6_hdr)];
                // OSPFv3
                reply_ospf->header.version = 3;
                // LS Update
                reply_ospf->header.type = 4;
                reply_ospf->header.length = htons(ospf_len);
                reply_ospf->header.router_id = htonl(router_id);
                reply_ospf->header.area_id = 0;
                reply_ospf->header.instance = 0;
                reply_ospf->header.zero = 0;

                // only one LSA
                reply_ospf->lsas = htonl(1);

                uint16_t ip_len = ospf_len + sizeof(ip6_hdr);
                reply_ip6->ip6_plen = htons(ospf_len);

                // compute checksum
                validateAndFillChecksum(output, ip_len);
                HAL_SendIPPacket(if_index, output, ip_len, src_mac);
                break;
              }
            }
          }

        } else if (ospf_hdr->type == 4 && ospf_hdr->version == 3) {
          // 如果是 OSPF LS Update Packet
          printf("Received OSPF LS Update from %s\n",
                 neighbor_router_id_buffer);
          ospf_ls_update *ospf = (ospf_ls_update *)&packet[sizeof(ip6_hdr)];

          if (neighbor_found) {
            // 把 LSA 保存到本地，使用 protocol-ospf 作业的函数进行解析
            const uint8_t *lsa_start;
            int lsa_num;
            OspfErrorCode err = parse_ip(packet, res, &lsa_start, &lsa_num);
            if (err != OspfErrorCode::SUCCESS) {
              printf("Failed to parse OSPF LS Update from %s: %s\n",
                     neighbor_router_id_buffer, ospf_error_to_string(err));
              continue;
            }

            // 遍历 LSA，同时收集 LSA Header
            int buf_len = res;
            std::vector<struct ospf_lsa_header> headers;
            for (int j = 0; j < lsa_num; j++) {
              uint16_t lsa_len;
              RouterLsa router_lsa;
              OspfErrorCode err =
                  disassemble(lsa_start, buf_len, &lsa_len, &router_lsa);
              struct ospf_lsa_header *lsa_header =
                  (struct ospf_lsa_header *)lsa_start;
              if (err == OspfErrorCode::SUCCESS ||
                  err == OspfErrorCode::ERR_LSA_NOT_ROUTER) {
                // 收集 LSA Header，用于组装 LS Acknowledgement
                headers.push_back(*lsa_header);

                // 把已经收到的 LSA 从 Link state request list 中删掉
                if (neighbor->state == NeighborLoading) {
                  auto &link_state_request_list =
                      neighbor->link_state_request_list;
                  for (int k = 0; k < link_state_request_list.size(); k++) {
                    if (link_state_request_list[k].ls_type ==
                            lsa_header->ls_type &&
                        link_state_request_list[k].link_state_id ==
                            lsa_header->link_state_id &&
                        link_state_request_list[k].advertising_router ==
                            lsa_header->advertising_router) {
                      link_state_request_list.erase(
                          link_state_request_list.begin() + k);
                      break;
                    }
                  }

                  // 如果 Link state request list 已经空了，转移状态到 Full
                  // RFC 2328 Page 87
                  // Loading Done
                  // "Link State Updates have been received for all
                  // out-of-date, portions of the database.  This is indicated
                  // by the Link state request list becoming empty after the
                  // Database Exchange process has completed."
                  //
                  // RFC 2328 Page 92
                  // "State(s):  Loading
                  // Event:  Loading Done
                  // New state:  Full"
                  if (link_state_request_list.empty()) {
                    // 和邻居路由器完成了 LSDB 同步，进入 Full 状态
                    printf("Neighbor %s enters Full state\n",
                           neighbor_router_id_buffer);
                    neighbor->state = NeighborFull;

                    // 更新 Router LSA，添加 neighbor
                    ospf_finish_sync(if_index, neighbor);
                  }
                }
              }

              if (
                  // Router LSA
                  err == OspfErrorCode::SUCCESS ||
                  // Intra-Area-Prefix LSA
                  (err == OspfErrorCode::ERR_LSA_NOT_ROUTER &&
                   ntohs(lsa_header->ls_type) == OSPF_INTRA_AREA_PREFIX_LSA)) {
                Lsa lsa;
                if (err == OspfErrorCode::SUCCESS) {
                  // Router LSA
                  lsa.type = OSPF_ROUTER_LSA;
                  lsa.router_lsa = router_lsa;
                } else {
                  // Intra-Area-Prefix LSA
                  lsa.type = OSPF_INTRA_AREA_PREFIX_LSA;

                  // 解析内容
                  lsa.intra_area_perfix_lsa.ls_age = ntohs(lsa_header->ls_age);
                  lsa.intra_area_perfix_lsa.link_state_id =
                      ntohl(lsa_header->link_state_id);
                  lsa.intra_area_perfix_lsa.advertising_router =
                      ntohl(lsa_header->advertising_router);
                  lsa.intra_area_perfix_lsa.ls_sequence_number =
                      ntohl(lsa_header->ls_sequence_number);

                  ospf_intra_area_prefix_lsa *intra_area_prefix_lsa =
                      (ospf_intra_area_prefix_lsa *)lsa_header;
                  lsa.intra_area_perfix_lsa.referenced_ls_type =
                      ntohs(intra_area_prefix_lsa->referenced_ls_type);
                  lsa.intra_area_perfix_lsa.referenced_link_state_id =
                      ntohl(intra_area_prefix_lsa->referenced_link_state_id);
                  lsa.intra_area_perfix_lsa.referenced_advertising_router =
                      ntohl(
                          intra_area_prefix_lsa->referenced_advertising_router);

                  int offset = sizeof(struct ospf_intra_area_prefix_lsa);
                  for (int k = 0; k < ntohs(intra_area_prefix_lsa->prefixes);
                       k++) {
                    struct ospf_intra_area_prefix_lsa_prefix_entry *entry =
                        (struct ospf_intra_area_prefix_lsa_prefix_entry
                             *)&lsa_start[offset];

                    IntraAreaPrefixLsaEntry new_entry;
                    new_entry.prefix_length = entry->prefix_length;
                    new_entry.prefix_options = entry->prefix_options;
                    new_entry.metric = ntohs(entry->metric);

                    // RFC 5340 A.4.1 IPv6 Prefix Representation
                    // "Address Prefix is an encoding of the prefix itself as
                    // an even multiple of 32-bit words, padding with zero
                    // bits as necessary.  This encoding consumes
                    // ((PrefixLength + 31) / 32) 32-bit words."
                    int words = (entry->prefix_length + 31) / 32;
                    int bytes = words * 4;
                    new_entry.address_prefix = in6_addr{0};
                    memcpy(&new_entry.address_prefix, &(lsa_start[offset + 4]),
                           bytes);

                    lsa.intra_area_perfix_lsa.entries.push_back(new_entry);
                    offset += bytes + 4;
                  }
                }

                // 把 LSA 更新到 lsdb
                if (lsdb.updateLSA(lsa)) {
                  // 需要 flood
                  ospf_flood_lsa(lsa);
                  // 更新路由表
                  lsdb.recalculateRoutingTable();
                }

                // RFC 2328 Page 145
                // "If the LSA is listed in the Link state retransmission
                // list for the receiving adjacency, the router itself is
                // expecting an acknowledgment for this LSA.  The router
                // should treat the received LSA as an acknowledgment by
                // removing the LSA from the Link state retransmission list.
                // This is termed an "implied acknowledgment".  Its
                // occurrence should be noted for later use by the
                // acknowledgment process"
                auto &link_state_retransmission_list =
                    neighbor->link_state_retransmission_list;
                for (int k = 0; k < link_state_retransmission_list.size();
                     k++) {
                  if (link_state_retransmission_list[k].type == lsa.type &&
                      link_state_retransmission_list[k].get_link_state_id() ==
                          lsa.get_link_state_id() &&
                      link_state_retransmission_list[k]
                              .get_advertising_router() ==
                          lsa.get_advertising_router()) {
                    printf(
                        "Remove acknowledged lsa from retransmission list\n");
                    link_state_retransmission_list.erase(
                        link_state_retransmission_list.begin() + k);
                    break;
                  }
                }
              } else {
                if (err != OspfErrorCode::ERR_LSA_NOT_ROUTER) {
                  printf("Failed to parse OSPF LS Update from %s: %s\n",
                         neighbor_router_id_buffer, ospf_error_to_string(err));
                }
                if (err == OspfErrorCode::ERR_PACKET_TOO_SHORT) {
                  break;
                }
              }

              buf_len -= lsa_len;
              lsa_start += lsa_len;
            }

            // 回复 OSPF LS Acknowledgement Packet
            ether_addr mac;
            HAL_GetInterfaceMacAddress(if_index, &mac);

            // IPv6 header
            ip6_hdr *reply_ip6 = (ip6_hdr *)&output[0];
            // flow label
            reply_ip6->ip6_flow = 0;
            // version
            reply_ip6->ip6_vfc = 6 << 4;
            // next header
            reply_ip6->ip6_nxt = 89;
            // hop limit
            reply_ip6->ip6_hlim = 1;
            // src ip
            reply_ip6->ip6_src = eui64(mac);
            // dst ip
            reply_ip6->ip6_dst = ip6->ip6_src;

            uint16_t ospf_len =
                sizeof(ospf_header) + sizeof(ospf_lsa_header) * headers.size();
            ospf_ls_acknowledgement *reply_ospf =
                (ospf_ls_acknowledgement *)&output[sizeof(ip6_hdr)];
            // OSPFv3
            reply_ospf->header.version = 3;
            // LS Acknowledgement
            reply_ospf->header.type = 5;
            reply_ospf->header.length = htons(ospf_len);
            reply_ospf->header.router_id = htonl(router_id);
            reply_ospf->header.area_id = 0;
            reply_ospf->header.instance = 0;
            reply_ospf->header.zero = 0;

            // copy lsa header from LS Update
            for (int j = 0; j < headers.size(); j++) {
              reply_ospf->lsa_header[j] = headers[j];
            }

            uint16_t ip_len = ospf_len + sizeof(ip6_hdr);
            reply_ip6->ip6_plen = htons(ospf_len);

            // compute checksum
            validateAndFillChecksum(output, ip_len);
            HAL_SendIPPacket(if_index, output, ip_len, src_mac);
          }
        } else if (ospf_hdr->type == 5 && ospf_hdr->version == 3) {
          // 如果是 OSPF LS Acknowledgement Packet
          printf("Received OSPF LS Acknowledgement from %s\n",
                 neighbor_router_id_buffer);

          ospf_ls_acknowledgement *ospf =
              (ospf_ls_acknowledgement *)&packet[sizeof(ip6_hdr)];

          int lsa_entries = (ntohs(ospf->header.length) - sizeof(ospf_header)) /
                            sizeof(ospf_lsa_header);
          // 检查 Link state retransmission list，如果出现匹配，则删掉对应的项目
          if (neighbor_found) {
            for (int j = 0; j < lsa_entries; j++) {
              auto &link_state_retransmission_list =
                  neighbor->link_state_retransmission_list;
              for (int k = 0; k < link_state_retransmission_list.size(); k++) {
                if (link_state_retransmission_list[k].type ==
                        ntohs(ospf->lsa_header[j].ls_type) &&
                    link_state_retransmission_list[k].get_link_state_id() ==
                        ntohl(ospf->lsa_header[j].link_state_id) &&
                    link_state_retransmission_list[k]
                            .get_advertising_router() ==
                        ntohl(ospf->lsa_header[j].advertising_router) &&
                    link_state_retransmission_list[k]
                            .get_ls_sequence_number() ==
                        ntohl(ospf->lsa_header[j].ls_sequence_number)) {
                  printf("Remove acknowledged lsa from retransmission list\n");
                  link_state_retransmission_list.erase(
                      link_state_retransmission_list.begin() + k);
                  break;
                }
              }
            }
          }
        }
      }

      continue;
    } else {
      // 目标地址不是我，考虑转发给下一跳
      // 检查是否是组播地址（ff00::/8），不需要转发组播分组
      if (ip6->ip6_dst.s6_addr[0] == 0xff) {
        printf("Don't forward multicast packet to %s\n",
               inet6_ntoa(ip6->ip6_dst));
        continue;
      }

      // 检查 TTL（Hop Limit）是否小于或等于 1
      uint8_t ttl = ip6->ip6_hops;
      if (ttl <= 1) {
        continue;
      } else {
        // 转发给下一跳
        // 按最长前缀匹配查询路由表
        in6_addr nexthop;
        uint32_t dest_if;
        if (prefix_query(ip6->ip6_dst, &nexthop, &dest_if)) {
          // 找到路由
          ether_addr dest_mac;
          // 如果下一跳为全
          // 0，表示的是直连路由，目的机器和本路由器可以直接访问
          if (nexthop == in6_addr{0}) {
            nexthop = ip6->ip6_dst;
          }
          if (HAL_GetNeighborMacAddress(dest_if, nexthop, &dest_mac) == 0) {
            // 在 NDP 表中找到了下一跳的 MAC 地址
            // TTL-1
            ip6->ip6_hops--;

            // 转发出去
            memcpy(output, packet, res);
            HAL_SendIPPacket(dest_if, output, res, dest_mac);
          } else {
            // 没有找到下一跳的 MAC 地址
            // 本实验中可以直接丢掉，等对方回复 NDP 之后，再恢复正常转发。
            printf("Nexthop ip %s is not found in NDP table\n",
                   inet6_ntoa(nexthop));
          }
        } else {
          // 没有找到路由
          printf("Destination IP %s not found in routing table",
                 inet6_ntoa(ip6->ip6_dst));
          printf(" and source IP is %s\n", inet6_ntoa(ip6->ip6_src));
        }
      }
    }
  }
  return 0;
}
