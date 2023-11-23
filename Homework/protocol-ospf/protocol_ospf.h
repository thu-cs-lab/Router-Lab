#ifndef _PROTOCOL_OSPF_H_
#define _PROTOCOL_OSPF_H_

#include "common.h"
#include <stdint.h>
#include <vector>

// disassemble 函数的返回值定义如下
// 如果同时出现多种错误，返回满足下面错误描述的第一条错误
// The return value of the disassemble function is defined as follows
// If multiple errors occur at the same time, the first error that satisfies the
// following error description is returned
enum OspfErrorCode {
  // 没有错误
  // No errors
  SUCCESS = 0,
  // 包长度太短，不足以完成解析；通常是因为包长度小于头部长度
  // Packet is too short to parse; usually because the packet length is less
  // than the header length
  ERR_PACKET_TOO_SHORT,
  // IPv6 头部的 next header 字段不为 OSPF
  // The next header field of the IPv6 header is not OSPF
  ERR_IPV6_NEXT_HEADER_NOT_OSPF,
  // 头部的 length 字段和包长度不匹配
  // The length field of the header does not match the packet length
  ERR_BAD_LENGTH,
  // OSPF 头部的 type 字段不为 LSU
  // The type field of the OSPF header is not LSU
  ERR_OSPF_NOT_LSU,
  // LSA 的校验和不正确
  // The checksum of the LSA is incorrect
  ERR_LSA_CHECKSUM,
  // LSA 的类型不是 Router-LSA
  // The type of LSA is not Router-LSA
  ERR_LSA_NOT_ROUTER,
  // LSA 的 age 字段大于 LSA_MAX_AGE
  // The age field of LSA is greater than LSA_MAX_AGE
  ERR_LS_AGE,
  // LSA 的 sequence number 字段为保留值
  // The sequence number field of LSA is a reserved value
  ERR_LS_SEQ,
  // Router-LSA 的项目不完整，即项目的总长度不是 16 的倍数
  // The Router-LSA entries are incomplete, that is, the total length of the
  // entries
  // is not a multiple of 16
  ERR_ROUTER_LSA_INCOMPLETE_ENTRY,
  // Router-LSA entry 的类型不正确，正确的类型应为 1 - 4
  // The type of Router-LSA entry is incorrect, and the correct type should be
  // 1 - 4
  ERR_ROUTER_LSA_ENTRY_TYPE,
  // 头部的 reserved 字段不为 0
  // The reserved field of the header is not 0
  ERR_BAD_ZERO,
};

static const char *ospf_error_to_string(OspfErrorCode err) {
  switch (err) {
  case SUCCESS:
    return "No errors";
  case ERR_PACKET_TOO_SHORT:
    return "Packet is too short";
  case ERR_IPV6_NEXT_HEADER_NOT_OSPF:
    return "IPv6 next header is not OSPF";
  case ERR_BAD_LENGTH:
    return "Packet length does not match the length field of header";
  case ERR_OSPF_NOT_LSU:
    return "OSPF packet is not LSU";
  case ERR_LSA_CHECKSUM:
    return "LSA checksum is incorrect";
  case ERR_LSA_NOT_ROUTER:
    return "LSA is not Router-LSA";
  case ERR_LS_AGE:
    return "LSA age is greater than LSA_MAX_AGE";
  case ERR_LS_SEQ:
    return "LSA sequence number is a reserved value";
  case ERR_ROUTER_LSA_INCOMPLETE_ENTRY:
    return "Router-LSA entry is incomplete";
  case ERR_ROUTER_LSA_ENTRY_TYPE:
    return "Router-LSA entry type is incorrect";
  case ERR_BAD_ZERO:
    return "Reserved bits are not zero";
  default:
    return "Unknown error code";
  }
}

// RFC 5340 A.3.1. The OSPF Packet Header
// https://datatracker.ietf.org/doc/html/rfc5340#appendix-A.3.1
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |   Version #   |     Type      |         Packet length         |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                         Router ID                             |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                          Area ID                              |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |          Checksum             |  Instance ID  |      0        |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
struct ospf_header {
  uint8_t version;    // Version #
  uint8_t type;       // Type
  uint16_t length;    // Packet length
  uint32_t router_id; // Router ID
  uint32_t area_id;   // Area ID
  uint16_t checksum;  // Checksum
  uint8_t instance;   // Instance ID
  uint8_t zero;       // 0
};

// RFC 5340 A.3.5. The Link State Update Packet
// https://datatracker.ietf.org/doc/html/rfc5340#appendix-A.3.5
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
//  # LSAs
//     The number of LSAs included in this update.
struct ospf_lsu_header {
  struct ospf_header header;
  uint32_t num_lsas;
};

// RFC 5340 A.4.2. The LSA Header
// https://datatracker.ietf.org/doc/html/rfc5340#appendix-A.4.2
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |           LS Age              |           LS Type             |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                       Link State ID                           |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                    Advertising Router                         |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                    LS Sequence Number                         |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |        LS Checksum            |             Length            |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
struct ospf_lsa_header {
  uint16_t ls_age;             // LS Age
  uint16_t ls_type;            // LS Type
  uint32_t link_state_id;      // Link State ID
  uint32_t advertising_router; // Advertising Router
  uint32_t ls_sequence_number; // LS Sequence Number
  uint16_t ls_checksum;        // LS Checksum
  uint16_t length;             // Length
};

// RFC 5340 A.4.3. Router-LSAs
// https://datatracker.ietf.org/doc/html/rfc5340#appendix-A.4.3
//  0                    1                   2                   3
//  0 1 2 3  4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |           LS Age               |0|0|1|         1               |
// +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                       Link State ID                            |
// +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                    Advertising Router                          |
// +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                    LS Sequence Number                          |
// +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |        LS Checksum             |            Length             |
// +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  0  |Nt|x|V|E|B|            Options                            |
// +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |     Type       |       0       |          Metric               |
// +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                      Interface ID                              |
// +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                   Neighbor Interface ID                        |
// +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                    Neighbor Router ID                          |
// +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             ...                                |
// +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |     Type       |       0       |          Metric               |
// +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                      Interface ID                              |
// +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                   Neighbor Interface ID                        |
// +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                    Neighbor Router ID                          |
// +-+-+-+--+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                             ...                                |
struct ospf_router_lsa_entry {
  uint8_t type;                   // Type
  uint8_t zero;                   // 0
  uint16_t metric;                // Metric
  uint32_t interface_id;          // Interface ID
  uint32_t neighbor_interface_id; // Neighbor Interface ID
  uint32_t neighbor_router_id;    // Neighbor Router ID
};

struct ospf_router_lsa {
  struct ospf_lsa_header header;
  uint8_t flags;    // 0 + Nt + x + V + E + B
  uint8_t zero;     // Options (high 8 bits)
  uint16_t options; // Options (low 16 bits)
  ospf_router_lsa_entry entries[];
};

// RFC 5340 A.3.1. The OSPF Packet Header
// https://datatracker.ietf.org/doc/html/rfc5340#appendix-A.3.1
// Type   Description
// ---------------------------------
// 1      Hello
// 2      Database Description
// 3      Link State Request
// 4      Link State Update
// 5      Link State Acknowledgment
enum OspfType {
  OSPF_HELLO = 1,
  OSPF_DD = 2,
  OSPF_LSR = 3,
  OSPF_LSU = 4,
  OSPF_LSAck = 5,
};

// RFC 2328 B. Architectural Constants
// https://lab.cs.tsinghua.edu.cn/router/doc/software/second_stage/static/rfc2328.html#appendix-B
// MaxAge
// The maximum age that an LSA can attain. When an LSA's LS age
// field reaches MaxAge, it is reflooded in an attempt to flush the
// LSA from the routing domain (See Section 14). LSAs of age MaxAge
// are not used in the routing table calculation.  The value of
// MaxAge is set to 1 hour.
const uint16_t LSA_MAX_AGE = 3600;

// RFC 2328 12.1.6. LS Sequence Number
// https://lab.cs.tsinghua.edu.cn/router/doc/software/second_stage/static/rfc2328.html#section-12.1.6
// The sequence number -N (0x80000000) is reserved (and
// unused).
const uint32_t RESERVED_LS_SEQ = 0x80000000;

struct RouterLsaEntry {
  uint8_t type;                   // Type
  uint16_t metric;                // Metric
  uint32_t interface_id;          // Interface ID
  uint32_t neighbor_interface_id; // Neighbor Interface ID
  uint32_t neighbor_router_id;    // Neighbor Router ID
};

struct RouterLsa {
  uint16_t ls_age;             // LS Age
  uint32_t link_state_id;      // Link State ID
  uint32_t advertising_router; // Advertising Router
  uint32_t ls_sequence_number; // LS Sequence Number
  uint8_t flags;               // 0 + Nt + x + V + E + B
  uint8_t zero;                // Options (high 8 bits)
  uint16_t options;            // Options (low 16 bits)
  std::vector<RouterLsaEntry> entries;
};

/*
  计算 OSPF LSA 的校验和（Fletcher checksum），输入一个 LSA
  和它的长度，返回计算结果。

  验证校验和时，不修改 LSA 中的
  checksum 字段，调用该函数，如果校验和正确，那么应当返回 0。

  生成校验和时，把 LSA 中的 checksum 字段设为
  0，调用该函数，就可以得到正确的校验和，再填入 checksum 字段。
*/
/*
  Compute checksum (Flectcher checksum) of OSPF LSA. Inputs a LSA entry and its
  length, returns the computation result.

  Upon checksum verification, do not modify checksum of LSA, but invoke this
  function. If the return value is zero, the checksum is correct.

  Upon checksum generation, set checksum of LSA to zero, and invoke this
  function to get the correct checksum, which should be filled to LSA checksum
  field afterwards.
*/
uint16_t ospf_lsa_checksum(struct ospf_lsa_header *lsa, size_t length);

/*
  你需要从 IPv6 分组中解析出 Router-LSA。一个 IPv6 分组中可能包含多个
  LSA，这些 LSA 并不都是 Router-LSA。你需要先解析 IPv6 分组头中的 OSPF
  头部和 LSU 头部，从中解析出 LSA 的起始位置和数量。然后，对每个 LSA，你需
  要判断其是否是 Router-LSA，然后从其长度推断 Entries 的数量，并解析出每
  个 Entry 的内容。
*/
/*
  You need to parse the Router-LSA from the IPv6 packet. An IPv6 packet may
  contain multiple LSAs, and these LSAs are not all Router-LSA. You need to
  first parse the OSPF header and LSU header in the IPv6 packet header to
  parse the starting position and number of LSAs. Then, for each LSA, you
  need to determine whether it is a Router-LSA, and then infer the number of
  Entries from its length, and parse the content of each Entry.
*/

/**
 * @brief 从接收到的 IPv6 分组解析出 LSA 的起始位置和数量
 * @param packet 接收到的 IPv6 分组
 * @param len 即 packet 的长度
 * @param lsa_start 用于返回 LSA 的起始位置
 * @param lsa_num 用于返回 LSA 的数量
 * @return 如果输入是一个合法的 LSU 报文，将 LSA 的起始位置和数量分别填入
 * lsa_start 和 lsa_num，并且返回 OspfErrorCode::SUCCESS；否则按照要求返回
 * OspfErrorCode 的具体类型
 *
 * 你需要按照以下顺序检查：
 * 1. len 是否能容纳 IPv6 header 的长度。
 * 2. IPv6 Header 中的 Payload Length 是否和 len 匹配。
 * 3. IPv6 Header 中的 Next header 字段是否为 OSPFv3 协议。
 * 4. len 是否能进一步容纳 OSPFv3 Header 的长度。
 * 5. OSPFv3 Header 中的 length 字段是否和 len 匹配。
 * 6. OSPFv3 Header 中的 type 字段是否为 LSU。
 * 7. len 是否能进一步容纳 LSU Header 的长度。
 */
/**
 * @brief Parse the starting position and number of LSAs from the received IPv6
 * packet
 * @param packet Received IPv6 packet
 * @param len The length of the packet
 * @param lsa_start Used to return the starting position of the LSA
 * @param lsa_num Used to return the number of LSAs
 * @return If the input is a legal LSU packet, fill in the starting position and
 * number of LSAs into lsa_start and lsa_num respectively, and return
 * OspfErrorCode::SUCCESS; otherwise, return the specific type of OspfErrorCode
 * as required
 *
 * You need to check in the following order:
 * 1. Whether len can accommodate the length of the IPv6 header.
 * 2. Whether the Payload Length in the IPv6 Header matches len.
 * 3. Whether the Next header field in the IPv6 Header is the OSPFv3 protocol.
 * 4. Whether len can accommodate the length of the OSPFv3 Header.
 * 5. Whether the length field in the OSPFv3 Header matches len.
 * 6. Whether the type field in the OSPFv3 Header is LSU.
 * 7. Whether len can accommodate the length of the LSU Header.
 */
OspfErrorCode parse_ip(const uint8_t *packet, uint32_t len,
                       const uint8_t **lsa_start, int *lsa_num);

/**
 * @brief 解析接收到的 LSA
 * @param lsa 接收到 LSA
 * @param buf_len 接收缓冲区中待处理数据的剩余长度。注意只要 buf_len 通过了检查，
 * 就应该将 LSA 的长度填入 len，即使 LSA 没有通过其它检查。
 * @param len 用于返回 LSA 的长度
 * @param output 用于返回解析出的 LSA
 * @return 如果输入是一个合法的 LSA，将 LSA 的长度和具体内容分别填入 len
 * 和 output，并且返回 OspfErrorCode::SUCCESS；否则按照要求返回 OspfErrorCode
 * 的具体类型。
 *
 * 你需要按照以下顺序检查：
 * 1. buf_len 是否能容纳 LSA header 的长度。
 * 2. buf_len 是否能容纳 LSA header 中宣称的 LSA 长度。
 * 3. LSA 头部中的 checksum 是否正确。（注意此处使用了一种特殊的 checksum
 * 计算方式，可以使用 ospf_lsa_checksum 函数来计算和验证）
 * 4. LSA 头部的 ls_age 是否超过了 LSA_MAX_AGE。
 * 5. LSA 头部的 ls_sequence_number 是否为保留值。
 * 6. LSA 类型是否为 Router-LSA。
 * 7. LSA 是否包含整数个 Entry。
 * 8. 每个 LSA Entry 是否具有合法的类型（1 - 4）。
 * 9. 每个 LSA Entry 中的 reserve 字段是否为 0。
 */
/**
 * @brief Parse the received LSA
 * @param lsa Received LSA
 * @param buf_len The remaining length of the received packet. Note that whenever
 * buf_len passes the checks, the value of len should be filled with the length
 * of the LSA, even if the LSA does not pass other checks.
 * @param len Used to return the length of the LSA
 * @param output Used to return the parsed LSA
 * @return If the input is a legal LSA, fill in the length and specific content
 * of the LSA into len and output respectively, and return
 * OspfErrorCode::SUCCESS; otherwise, return the specific type of OspfErrorCode
 * as required.
 *
 * You need to check in the following order:
 * 1. Whether buf_len can accommodate the length of the LSA header.
 * 2. Whether buf_len can accommodate the length of the LSA claimed in the LSA
 * header.
 * 3. Whether the checksum in the LSA header is correct. (Note that a special
 * checksum calculation method is used here, you can use ospf_lsa_checksum to
 * compute and verify)
 * 4. Whether the ls_age in the LSA header exceeds LSA_MAX_AGE.
 * 5. Whether the ls_sequence_number in the LSA header is a reserved value.
 * 6. Whether the type of LSA is Router-LSA.
 * 7. Whether the LSA contains an integer number of Entries.
 * 8. Whether each LSA Entry has a valid type (1 - 4).
 * 9. Whether the reserve field in each LSA Entry is 0.
 */
OspfErrorCode disassemble(const uint8_t *lsa, uint16_t buf_len, uint16_t *len,
                          RouterLsa *output);

#endif // _PROTOCOL_OSPF_H_