# protocol

## Problem Description

This problem requires you to implement the [RFC 2080: RIPng for IPv6](https://datatracker.ietf.org/doc/html/rfc2080) data format processing. You need to implement the following two functions, as annotated in the functions in the ``protocol.cpp`` file.

```cpp
// See the code for the function comments
RipngErrorCode disassemble(const uint8_t *packet, uint32_t len, RipngPacket *output) {
  // TODO
  return RipngErrorCode::SUCCESS;
}

// See code for function comments
uint32_t assemble(const RipngPacket *ripng, uint8_t *buffer) {
  // TODO
  return 0;
}
```

You need to implement.

1. parse the IPv6, UDP and RIPng data and save the useful information
2. recover the contents of RIPng from the saved information if you got the legal data in the previous step

Note that all data in this problem is in network transmission format, so each RIPng data field is in network byte order, see the note in `rip.h`.

The data used for the evaluation are in the `data` directory. The input has a total of $n$ IPv6 packets, and `main.cpp` calls your code to determine if each packet is a legitimate RIPng message, and if so, saves it and outputs a line `Valid {numEntries} {command}`, followed by `numEntries` lines, each of which corresponds to `prefix_or_nh` `route_tag` `prefix_len` and `metric`, and then a line of output, which is the reconstructed RIPng message, in hexadecimal format; if it is not legal, the illegal part is printed.

## Sample 1

See *protocol_input1.pcap* and *protocol_answer1.txt* in the data directory.

## Example 1 Explanation

Open it with Wireshark and you can see that there are two legitimate RIPng packets, which look like this

```text
RIPng
    Command: Request (1)
    Version: 1
    Reserved: 0000
RIPng
    Command: Response (2)
    Version: 1
    Reserved: 0000
```

You can see that they are all legal RIPng packets, just fill the corresponding contents into the structure, and pay attention to the byte order.
