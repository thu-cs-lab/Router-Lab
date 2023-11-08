# protocol

## Problem Description

This problem requires you to implement the [RFC 5340: OSPF for IPv6](https://datatracker.ietf.org/doc/html/rfc5340/) data format processing. You need to implement the following two functions in `protocol_ospf.cpp`, as annotated in the functions in the `protocol_ospf.h` file:

```cpp
// See protocol_ospf.h for the function comments
OspfErrorCode parse_ip(const uint8_t *packet, uint32_t len, const uint8_t **lsa_start, int *lsa_num) {
  // TODO
  return OspfErrorCode::SUCCESS;
}

// See protocol_ospf.h for the function comments
OspfErrorCode disassemble(const uint8_t* lsa, uint16_t buf_len, uint16_t *len, RouterLsa *output) {
  // TODO
  return OspfErrorCode::SUCCESS;
}
```

You need to implement:

1. parse the IPv6, OSPF, and LSU (Link State Update) packet formats to get the starting position and number of LSAs (Link State Advertisements)
2. if the previous step gets legal data, parse the LSAs

Note that all data in this problem is in network transmission format, so each OSPF data field is in network byte order.

The data used for the judgment are in the `data` directory. The input has a total of $n$ IPv6 packets, and `main.cpp` calls your code to determine if each packet is a legitimate OSPF packet. If it is legal, save it and output a line `Success`. After that, there are `lsa_num` groups, each group first outputs the LSA type and the corresponding LSA length. If the LSA is a Router-LSA, then further output its `ls_age` `link_state_id` `advertising_router` and `ls_sequence_number`. If the Router-LSA contains some Entries, for each Entry, first output a line `Entry`, and then further output the corresponding `type` `metric` `interface_id` `neighbor_interface_id` `neighbor_router_id`. If it is not legal, output the illegal part.

## Sample 1

See *protocol_ospf_input1.pcap* and *protocol_ospf_answer1.txt* in the data directory.

## Sample 1 Explanation

Open it with Wireshark and you can see that there is a legitimate OSPF packet, which is an LSU packet. This LSU packet includes 10 LSAs, two of which are Router-LSAs. Therefore, you only need to fill in the information in these two Router-LSAs into the corresponding structure.