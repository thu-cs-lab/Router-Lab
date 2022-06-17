#include "protocol.h"
#include "common.h"
#include "lookup.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

RipngErrorCode disassemble(const uint8_t *packet, uint32_t len,
                         RipngPacket *output) {
  // TODO
  return RipngErrorCode::SUCCESS;
}

uint32_t assemble(const RipngPacket *ripng, uint8_t *buffer) {
  // TODO
  return 0;
}