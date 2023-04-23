# eui64

## Problem Description

In IPv6, Link Local addresses become more important. In order to generate a Link Local address that will not conflict, there is a simple way (EUI-64) to convert to a Link Local IPv6 address by using the uniqueness of MAC addresses as follows.

1. input a 48-bit MAC address and output a 128-bit IPv6 address
2. set the first byte of the IPv6 address to 0xFE, the second byte to 0x80, and the third to eighth bytes to 0. This step will make the IPv6 address a Link Local address.
3. divide the MAC address into two halves to be written into the IPv6 address: copy the 1st to 3rd byte of the MAC address to the 9th to 11th byte of the IPv6 address; then copy the 4th to 6th byte to the 14th to 16th byte of the IPv6 address.
4. Set the 12th byte of the IPv6 address to 0xFF and the 13th byte to 0xFE to fill the two empty bytes in between.
5. Invert the 9th byte of the IPv6 address in the network bit order from left to right bit 7 (see the diagram in RFC 4291).

This procedure is defined in [RFC 4291](https://datatracker.ietf.org/doc/html/rfc4291).

You need to implement this conversion process in the `eui64.cpp` file in the following functions.

```cpp
// See comments in the code for the function
in6_addr eui64(const ether_addr mac) {
  in6_addr res = {0};
  // TODO
  return res;
}
```

The `in6_addr` type holds 128-bit address information for IPv6, please read and write through the `uint8_t s6_addr[16]` member, e.g. `res.s6_addr[0] = 0xfe;`. Similarly, the `ether_addr` type is read and written via the `uint8_t ether_addr_octet[6]` member. Using other way of writing (e.g. other names in macOS structs) will result in compilation errors in CI.

You don't need to handle the input and output, you just need to execute `make grade` locally for local evaluation.

## Input and output file format

The input file has a number of lines, one MAC address per line.

The output file has several lines, one IPv6 address per line, generated using the EUI-64 algorithm for each MAC address line in the input file.

The IPv6 address is converted to a string using the `inet_ntop` function. In short, each of the 16 bytes of the IPv6 address is converted to hexadecimal, in groups of two, separated by `:`; if there are consecutive 0000s, then they can be omitted. See [Wikipedia](https://en.wikipedia.org/wiki/IPv6_address#Representation) for the full rules.

## Sample 1

See *eui64_input1.txt* and *eui64_answer1.txt* in the data directory.

## Example 1 Explanation

The input of sample 1 has only one MAC address: 11:22:33:44:55:66, and is processed according to the requested algorithm.

1. set the first 8 bytes of the IPv6 address to get fe80:0000:0000:0000:0000:0000:0000:0000.
2. divide the MAC address into two parts and fill them in to get fe80:0000:0000:0000:1122:3300:0044:5566.
3. Fill in the empty space in the middle to get fe80:0000:0000:0000:1122:33ff:fe44:5566.
4. Invert the low to high 2 bits of 11, i.e. 13, to get fe80:0000:0000:0000:1322:33ff:fe44:5566.
5. The `main` function uses the `inet_ntop` function to output the IPv6 address, omitting the consecutive zeros as ::, to get the final result fe80::1322:33ff:fe44:5566.
