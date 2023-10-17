#include "protocol_ospf.h"
#include <stdint.h>

uint16_t ospf_lsa_checksum(struct ospf_lsa_header *lsa, size_t length) {
  // RFC 905 B.3  ALGORITHM FOR GENERATING CHECKSUM PARAMETERS & B.4  ALGORITHM
  // FOR CHECKING CHECKSUM PARAMETERS
  uint8_t *buff = (uint8_t *)&lsa->ls_type;
  length -= buff - (uint8_t *)lsa;

  // "Addition is performed in one of the two following modes:
  // a)  modulo 255 arithmetic;
  // b)  one's  complement  arithmetic  in  which  if  any  of  the
  //     variables  has the value minus zero (i.e. 255) it shall be
  //     regarded as though it was plus zero (i.e. 0)."

  // For generation:
  // "B.3 Set up the complete TPDU with the value  of  the  checksum
  // parameter field set to zero."

  // "Initialize C0 and C1 to zero."
  uint16_t c0 = 0, c1 = 0;
  // "n   number (i.e. position) of the first octet of the  checksum
  //      parameter"
  size_t n = (uint8_t *)&lsa->ls_checksum - buff + 1;

  // "Process each octet sequentially from i = 1 to L by:
  // a)  adding the value of the octet to C0; then
  // b)  adding the value of C0 to C1."
  for (size_t i = 0; i < length; ++i) {
    c0 += buff[i];
    c0 = (c0 >> 8) + (c0 & 0xff);
    c1 += c0;
    c1 = (c1 >> 8) + (c1 & 0xff);
  }

  // "Calculate X and Y such that
  // X = -C1 + (L-n).CO
  // Y =  C1 - (L-n+1).C0"
  uint32_t x = (length - n) * c0;
  x = (x >> 16) + (x & 0xffff);
  x = (x >> 16) + (x & 0xffff);
  x = (x >> 8) + (x & 0xff);
  x = (x >> 8) + (x & 0xff);
  x += ~c1 & 0xff;
  x = (x >> 8) + (x & 0xff);
  if (x == 0xff)
    x = 0;
  uint32_t y = (length - n + 1) * c0;
  y = (y >> 16) + (y & 0xffff);
  y = (y >> 16) + (y & 0xffff);
  y = (y >> 8) + (y & 0xff);
  y = (y >> 8) + (y & 0xff);
  y = c1 + (~y & 0xff);
  y = (y >> 8) + (y & 0xff);
  if (y == 0xff)
    y = 0;

  // B.3
  // "Place the values  X  and  Y  in  octets  n  and  (n  +  1)
  // respectively.""
  // For generation: save X and Y in LSA checksum

  // B.4
  // "If, when all the octets have  been  processed,  either  or
  // both  of  C0  and  C1  does not have the value zero, the checksum
  // formulas in 6.17 have not been satisfied."
  // For verification: check if X == 0 and Y == 0
  return x | y << 8;
}