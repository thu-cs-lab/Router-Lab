#include "protocol.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

RipErrorCode disassemble(const uint8_t *packet, uint32_t len,
                         RipPacket *output) {
  // TODO
  return RipErrorCode::SUCCESS;
}

uint32_t assemble(const RipPacket *rip, uint8_t *buffer) {
  // TODO
  return 0;
}