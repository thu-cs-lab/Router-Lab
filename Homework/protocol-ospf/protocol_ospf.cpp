#include "protocol_ospf.h"
#include "common.h"
#include "lookup.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

OspfErrorCode parse_ip(const uint8_t *packet, uint32_t len,
                       const uint8_t **lsa_start, int *lsa_num) {
  // TODO
  return OspfErrorCode::SUCCESS;
}

OspfErrorCode disassemble(const uint8_t *lsa, uint16_t buf_len, uint16_t *len,
                          RouterLsa *output) {
  // TODO
  return OspfErrorCode::SUCCESS;
}