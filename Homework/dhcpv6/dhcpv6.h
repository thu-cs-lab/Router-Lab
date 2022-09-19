#include <stdint.h>

struct dhcpv6_hdr {
  // 1-byte message type
  uint8_t msg_type;
  // 3-byte transaction id
  uint8_t transaction_id_hi;
  uint16_t transaction_id_lo;
  // options follow
};