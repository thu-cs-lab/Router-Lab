#include <stdint.h>

// disassemble 函数的返回值定义如下
// 如果同时出现多种错误，返回满足下面错误描述的第一条错误
enum RipErrorCode {
  // 没有错误
  SUCCESS = 0,
  // IP 头、UDP 头和实际的 RIP 路由项的长度不一致
  ERR_LENGTH,
  // IP 头中 proto 字段不是 UDP 协议
  ERR_IP_PROTO_NOT_UDP,
  // UDP 头中端口号不是 520(rip)
  ERR_BAD_UDP_PORT,
  // RIP 的 Command 字段错误
  ERR_RIP_BAD_COMMAND,
  // RIP 的 Version 字段错误
  ERR_RIP_BAD_VERSION,
  // RIP 的 Zero 字段错误
  ERR_RIP_BAD_ZERO,
  // RIP 表项的 Family 字段错误
  ERR_RIP_BAD_FAMILY,
  // RIP 表项的 Metric 字段错误
  ERR_RIP_BAD_METRIC,
  // RIP 表项的 Mask 字段错误
  ERR_RIP_BAD_MASK,
  // RIP 表项的 Mask 字段和 Address 字段不符合要求
  ERR_RIP_MASK_ADDRESS_INCONSISTENT,
};

#define RIP_MAX_ENTRY 25
typedef struct {
  // 所有字段都是网络字节序（大端序）
  // 没有存储 `family` 字段，因为在请求里是 0，在回应里是 2
  // 也不用存储 `tag` 字段，因为它是 0
  uint32_t addr;
  uint32_t mask;
  uint32_t nexthop;
  uint32_t metric;
} RipEntry;

typedef struct {
  uint32_t numEntries;
  // 下面所有字段都是网络字节序（大端序）
  uint8_t command;
  // 不用存储 `version`，因为它总是 2
  // 不用存储 `zero`，因为它总是 0
  RipEntry entries[RIP_MAX_ENTRY];
} RipPacket;

static const char *rip_error_to_string(RipErrorCode err) {
  switch (err) {
  case RipErrorCode::ERR_LENGTH:
    return "Length is inconsistent";
  case RipErrorCode::ERR_IP_PROTO_NOT_UDP:
    return "IP proto field is not UDP";
  case RipErrorCode::ERR_BAD_UDP_PORT:
    return "UDP port is not 520";
  case RipErrorCode::ERR_RIP_BAD_COMMAND:
    return "Command field is bad";
  case RipErrorCode::ERR_RIP_BAD_VERSION:
    return "Version field is bad";
  case RipErrorCode::ERR_RIP_BAD_ZERO:
    return "Zero field is bad";
  case RipErrorCode::ERR_RIP_BAD_FAMILY:
    return "Family field is bad";
  case RipErrorCode::ERR_RIP_BAD_METRIC:
    return "Metric field is bad";
  case RipErrorCode::ERR_RIP_BAD_MASK:
    return "Mask field is bad";
  case RipErrorCode::ERR_RIP_MASK_ADDRESS_INCONSISTENT:
    return "Address field is inconsistent with Mask field";
  default:
    return "Unknown error code";
  }
}