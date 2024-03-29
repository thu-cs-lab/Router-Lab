#include "common.h"
#include <stdint.h>

// disassemble 函数的返回值定义如下
// 如果同时出现多种错误，返回满足下面错误描述的第一条错误
// The return value of the disassemble function is defined as follows
// If multiple errors occur at the same time, the first error that satisfies the
// following error description is returned
enum RipngErrorCode {
  // 没有错误
  // No errors
  SUCCESS = 0,
  // IPv6 头中 next header 字段不是 UDP 协议
  // The next header field in the IPv6 header is not UDP protocol
  ERR_IPV6_NEXT_HEADER_NOT_UDP,
  // UDP 头中源端口号或者目的端口号不是 521(RIPng)
  // The source or destination port number in the UDP header is not 521 (RIPng)
  ERR_UDP_PORT_NOT_RIPNG,
  // IPv6 头、UDP 头和实际的 RIPng 路由项的长度出现错误或者不一致
  // The length of the IPv6 header, UDP header and the actual RIPng routing
  // entry are incorrect or do not match
  ERR_LENGTH,
  // RIPng 的 Command 字段错误
  // The Command field of RIPng is wrong
  ERR_RIPNG_BAD_COMMAND,
  // RIPng 的 Version 字段错误
  // The Version field of RIPng is wrong
  ERR_RIPNG_BAD_VERSION,
  // RIPng 的 Zero（Reserved）字段错误
  // The Zero(Reserved) field of RIPng is wrong
  ERR_RIPNG_BAD_ZERO,
  // RIPng 表项的 Metric 字段错误
  // Wrong Metric field in RIPng table entry
  ERR_RIPNG_BAD_METRIC,
  // RIPng 表项的 Prefix Len 字段错误
  // Wrong Prefix Len field in RIPng table entry
  ERR_RIPNG_BAD_PREFIX_LEN,
  // RIPng 表项的 Route Tag 字段错误
  // Wrong Route Tag field in RIPng table entry
  ERR_RIPNG_BAD_ROUTE_TAG,
  // RIPng 表项的 Prefix 和 Prefix Len 字段不符合要求
  // The Prefix and Prefix Len fields of the RIPng table entry are inconsistent
  ERR_RIPNG_INCONSISTENT_PREFIX_LENGTH,
};

// RIPng header 定义
// RIPng header definition
// https://datatracker.ietf.org/doc/html/rfc2080#page-5
// "The RIPng packet format is:"
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  command (1)  |  version (1)  |       must be zero (2)        |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                                                               |
// ~                Route Table Entry 1 (20)                       ~
// |                                                               |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                                                               |
// ~                         ...                                   ~
// |                                                               |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                                                               |
// ~                Route Table Entry N (20)                       ~
// |                                                               |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
typedef struct ripng_hdr {
  // 1 表示 request，2 表示 response
  // 1 for request, 2 for response
  uint8_t command;
  // 应当为 1
  // should be 1
  uint8_t version;
  // 应当为 0
  // should be 0
  uint16_t zero;
} ripng_hdr;

// (1500-40-8-4)/20=72
// https://datatracker.ietf.org/doc/html/rfc2080#page-7
// "The maximum datagram size is limited by the MTU of the medium over
// which the protocol is being used.  Since an unsolicited RIPng update
// is never propagated across a router, there is no danger of an MTU
// mismatch.  The determination of the number of RTEs which may be put
// into a given message is a function of the medium's MTU, the number of
// octets of header information preceeding the RIPng message, the size
// of the RIPng header, and the size of an RTE.  The formula is:"
//             +-                                                   -+
//             | MTU - sizeof(IPv6_hdrs) - UDP_hdrlen - RIPng_hdrlen |
// #RTEs = INT | --------------------------------------------------- |
//             |                      RTE_size                       |
//             +-                                                   -+
#define RIPNG_MAX_RTE 72

// RIPng entry 定义
// RIPng entry definition
// https://datatracker.ietf.org/doc/html/rfc2080#page-6
// "Route Table Entry (RTE) has the following format:"
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                                                               |
// ~                        IPv6 prefix (16)                       ~
// |                                                               |
// +---------------------------------------------------------------+
// |         route tag (2)         | prefix len (1)|  metric (1)   |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
typedef struct {
  // 当 Metric=0xFF 时记录了 Nexthop；当 Metric!=0xFF 时记录了 Prefix
  // Nexthop is recorded when Metric=0xFF; Prefix is recorded when Metric!=0xFF
  // "A next hop RTE is identified by a value of 0xFF in the metric field
  // of an RTE.  The prefix field specifies the IPv6 address of the next
  // hop.  The route tag and prefix length in the next hop RTE must be set
  // to zero on sending and ignored on receiption."
  in6_addr prefix_or_nh;
  // 网络字节序存储
  // Saved in network byte order
  uint16_t route_tag;
  uint8_t prefix_len;
  uint8_t metric;
} ripng_rte;

typedef struct {
  uint32_t numEntries;
  // 1 表示 request，2 表示 response
  // 1 for request, 2 for response
  uint8_t command;
  // 不用存储 `version`，因为它总是 1
  // No need to store `version`, because it is always 1
  // 不用存储 `zero`，因为它总是 0
  // no need to store `zero`, because it is always 0
  ripng_rte entries[RIPNG_MAX_RTE];
} RipngPacket;

static const char *ripng_error_to_string(RipngErrorCode err) {
  switch (err) {
  case RipngErrorCode::ERR_IPV6_NEXT_HEADER_NOT_UDP:
    return "IP next header field is not UDP";
  case RipngErrorCode::ERR_UDP_PORT_NOT_RIPNG:
    return "UDP port is not 521";
  case RipngErrorCode::ERR_LENGTH:
    return "Length is inconsistent";
  case RipngErrorCode::ERR_RIPNG_BAD_COMMAND:
    return "Command field is bad";
  case RipngErrorCode::ERR_RIPNG_BAD_VERSION:
    return "Version field is bad";
  case RipngErrorCode::ERR_RIPNG_BAD_ZERO:
    return "Zero(Reserved) field is bad";
  case RipngErrorCode::ERR_RIPNG_BAD_METRIC:
    return "Metric field is bad";
  case RipngErrorCode::ERR_RIPNG_BAD_PREFIX_LEN:
    return "Prefix len field is bad";
  case RipngErrorCode::ERR_RIPNG_BAD_ROUTE_TAG:
    return "Route tag field is bad";
  case RipngErrorCode::ERR_RIPNG_INCONSISTENT_PREFIX_LENGTH:
    return "Prefix field is inconsistent with Prefix len field";
  default:
    return "Unknown error code";
  }
}

/*
  你需要从 IPv6 packet 中解析出 RipngPacket 结构体，也要从 RipngPacket
  结构体构造出对应的 IPv6 packet。 由于 RIPng 本身不记录表项的个数，需要从 IPv6
  header 的长度中推断，所以在 RipngPacket 中额外记录了个数。
*/
/*
  You need to extract the RipngPacket structure from the IPv6 packet, and also
  construct the corresponding IPv6 packet from the RipngPacket. Since RIPng
  itself does not record the number of table entries, you need to infer it from
  the length of the IPv6 header, so the number is additionally recorded in the
  RipngPacket.
*/

/**
 * @brief 从接受到的 IPv6 packet 解析出 RIPng 协议的数据
 * @param packet 接受到的 IPv6 分组
 * @param len 即 packet 的长度
 * @param output 把解析结果写入 *output
 * @return 如果输入是一个合法的 RIPng 报文，把它的内容写入 RipngPacket 并且返回
 * RipngErrorCode::SUCCESS；否则按照要求返回 RipngErrorCode 的具体类型
 *
 * 你需要按照以下顺序检查：
 * 1. len 是否不小于一个 IPv6 header 的长度。
 * 2. IPv6 Header 中的 Payload Length 加上 Header 长度是否等于 len。
 * 3. IPv6 Header 中的 Next header 字段是否为 UDP 协议。
 * 4. IPv6 Header 中的 Payload Length 是否包括一个 UDP header 的长度。
 * 5. 检查 UDP 源端口和目的端口是否都为 521。
 * 6. 检查 UDP header 中 Length 是否等于 UDP header 长度加上 RIPng header
 * 长度加上 RIPng entry 长度的整数倍。
 * 7. 检查 RIPng header 中的 Command 是否为 1 或 2，
 * Version 是否为 1，Zero（Reserved） 是否为 0。
 * 8. 对每个 RIPng entry，当 Metric=0xFF 时，检查 Prefix Len
 * 和 Route Tag 是否为 0。
 * 9. 对每个 RIPng entry，当 Metric!=0xFF 时，检查 Metric 是否属于
 * [1,16]，并检查 Prefix Len 是否属于 [0,128]，Prefix Len 是否与 IPv6 prefix
 * 字段组成合法的 IPv6 前缀。
 */
/**
 * @brief Parse the RIPng protocol data from the received IPv6 packet
 * @param packet The received IPv6 packet
 * @param len the length of the packet
 * @param output writes the parsed result to *output
 * @return If the input is a legitimate RIPng message, write its content to the
 * RipngPacket and return RipngErrorCode::SUCCESS; otherwise return the specific
 * type of RipngErrorCode as required
 *
 * You need to check in the following order:
 * 1. Whether len is not less than the length of an IPv6 header.
 * 2. Whether the Payload Length plus the Header Length in the IPv6 Header is
 * equal to len.
 * 3. Whether the Next header field in the IPv6 Header is UDP protocol.
 * 4. Whether the Payload Length in the IPv6 Header includes the length of a UDP
 * header.
 * 5. Check if the UDP source port and destination port are both 521.
 * 6. Check that the Length in the UDP header is equal to the UDP header length
 * plus the RIPng header length plus the integer multiple of the RIPng entry
 * length.
 * 7. Check if Command in the RIPng header is 1 or 2. Version is 1, and Zero
 * (Reserved) is 0.
 * 8. For each RIPng entry, when Metric=0xFF, check if Prefix Len
 * and Route Tag is 0.
 * 9. For each RIPng entry, when Metric!=0xFF, check if Metric belongs to
 * [1,16], and check if Prefix Len belongs to [0,128], and check if Prefix Len
 * and IPv6 prefix form a legal IPv6 prefix.
 */
RipngErrorCode disassemble(const uint8_t *packet, uint32_t len,
                           RipngPacket *output);

/**
 * @brief 从 RipngPacket 的数据结构构造出 RIPng 协议的二进制格式
 * @param ripng 一个 RipngPacket 结构体
 * @param buffer 一个足够大的缓冲区，你要把 RIPng 协议的数据写进去
 * @return 写入 buffer 的数据长度
 *
 * 在构造二进制格式的时候，你需要把 RipngPacket 中没有保存的一些固定值补充上，
 * 包括 Version 和 Zero 字段。
 * 你写入 buffer 的数据长度（同时也是该函数的返回值）都应该是四个字节（RIPng
 * 头）加上每个 RIPng 表项 20 字节。需要注意一些没有保存在 RipngPacket
 * 结构体内的数据的填写。
 */
/**
 * @brief Constructs the binary format of the RIPng protocol from the data
 * structure of the RipngPacket
 * @param ripng a RipngPacket structure
 * @param buffer a buffer large enough for you to write the RIPng protocol data
 * into
 * @return the length of the data written to the buffer
 *
 * When constructing the binary format, you need to add some fixed values that
 * are not stored in the RipngPacket, including the Version and Zero fields. The
 * length of the data you write to the buffer (which is also the return value of
 * the function) should be four bytes (the RIPng header) plus 20 bytes for each
 * RIPng table entry. Note that some data to be written is not stored in the
 * RipngPacket structure.
 */
uint32_t assemble(const RipngPacket *ripng, uint8_t *buffer);