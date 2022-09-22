#include <stdint.h>

struct tftp_hdr {
  // 2-byte opcode
  uint16_t opcode;
  union {
    // opcode = DATA(0x3) or ACK(0x4):
    // 2-byte block number
    uint16_t block_number;
    // opcode = ERROR(0x5):
    // 2-byte error code
    uint16_t error_code;
  };
};