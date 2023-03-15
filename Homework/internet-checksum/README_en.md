# internet-checksum

## Problem Description

You need to implement the following function `validateAndFillChecksum` in `checksum.cpp`, which takes an IPv6 packet, which after the IPv6 Header must be a UDP or ICMPv6 packet. The function returns whether the checksum is correct or not; also, the checksum in the packet should be filled with the correct value when the function returns, regardless of whether the original checksum is correct or not.

```cpp
// See the code for function comments
bool validateAndFillChecksum(uint8_t *packet, size_t len) {
  return true;
}
```

In IPv6, the Header no longer has a checksum field, but when calculating the checksum in UDP and ICMPv6 protocols, a part of the IPv6 Header needs to be taken into account, which is the Pseudo Header, consisting of the following parts.

1. 16-byte Source IPv6 Address
2. 16-byte Destination IPv6 Address
3. 4 bytes of UDP/ICMPv6 Length (don't forget to use the network byte order)
4. 3 bytes of 0, followed by 1 byte of Next Protocol (17 for UDP, 58 for ICMPv6)

If you are still confused about this part, you can compare [UDP Checksum](https://en.wikipedia.org/wiki/User_Datagram_Protocol#IPv6_pseudo_header) and [ICMPv6 Checksum](https://en) .wikipedia.org/wiki/Internet_Control_Message_Protocol_for_IPv6#Checksum) tables on the web page for implementation.

The checksum is verified as follows.

1. Splice the UDP/ICMPv6 packet after the IPv6 Pseudo Header. The spliced data is grouped every 16 binary bits and treated as a big-endian integer. Sum all 16-bit integers.
2. If the above summation result overflows (more than 16 bits in binary representation), split it into the lower 16 bits and the overflow part, and then add the overflow part to the lower 16 bits (e.g., if the summation result is 0x1CB2F, split it into the lower 16 bits of 0xCB2F and the overflow part 0x1, and add 0xCB2F + 0x1).
3. If the result overflows again, the above operation is repeated until no overflow occurs.
4. If the above result is equal to 0xFFFF, the checksum is correct and vice versa.

In particular, in UDP, a checksum field of 0 means that no checksum calculation is performed. However, due to the absence of a checksum field in the IPv6 header, etc., UDP over IPv6 requires that checksum computation must be performed. Therefore, for UDP, if the receiver finds a checksum field of 0 when checking, it should be considered as an error.

Since a byte occupies 8 binary bits, the "every 16 binary bits" in step 1 of the checksum check may face insufficient data, for example, if the UDP/ICMPv6 packet length is an odd number of bytes (the length of the IPv6 Pseudo Header is fixed at 40 bytes). It is a feasible way to handle the splicing result in step 1 by adding zero to an even number of bytes, and the same is done in the checksum calculation below.

The checksum is calculated as follows.

1. Change the checksum field to 0.
2. Perform steps 1, 2 and 3 in the checksum check to obtain the 16-bit result.
3. Invert the 16-bit result by bit and fill the checksum field using big endian to complete the checksum calculation.

As with the special handling of UDP in checksum checking, since a checksum of 0 in UDP has a special meaning, if the checksum calculated by the sender is indeed 0x0000, it needs to be set to 0xFFFF as well to show the difference. For other cases (only ICMPv6 here, of course), when the checksum is accepted as 0x0000 but is actually 0xFFFF, the return value of the function is set to true (indicating successful checksum) and the checksum in the packet is set to 0x0000 (changed to the correct one).

Please analyze the correctness of the above two special handling practices: will the above practices confuse the two cases of checksum 0x0000 and 0xFFFF? Does the change to the UDP checksum pass the checksum test? And why the special treatment for the case of "0x0000 when it should be 0xFFFF"? If you have doubts about this part, you can refer to the analysis of Article 2 in the "Notes and Remarks" below.

Notes and Remarks:

1. In the above description of checksum, we first give the checksum check method and then the checksum calculation method, which is intentional. Please note that step 1 of the checksum calculation method is not simply the initial value of the checksum calculation, but an actual operation that needs to be done: changing the UDP/ICMPv6 checksum field to 0. After this step is done, the checksum check cannot be performed. Therefore, we suggest that students can follow the above process and write it step by step, even though the process may be slightly awkward.

2. Further discussion on the correctness of the special treatment in the checksum calculation: There is a property in the checksum calculation and checking method that the checksum obtained according to the algorithm of the calculation must be the correct result from the check, but not vice versa. We can observe that the result of the checksum calculation cannot be 0xFFFF: because the result of the summation in step 2 of the calculation cannot be 0x0000, the final checksum cannot be 0xFFFF after the inversion in step 3, but if the checksum is equal to 0xFFFF, the result obtained by the algorithm for checking the correct checksum is still correct. You can think why replacing 0x0000 with 0xFFFF or vice versa in this operation does not affect the summation result, which explains why it is correct to change the checksum of 0x0000 to 0xFFFF in UDP.

   You may ask, if everyone uses the above calculation method to get the checksum, why does the 0xFFFF situation still occur? Does the case of "0xFFFF when it should be 0x0000" exist? The incremental update algorithm defined in [RFC 1141](https://datatracker.ietf.org/doc/html/rfc1141), due to a design error, results in a checksum of 0xFFFF, which is later fixed in [RFC 1624](https://datatracker.ietf.org/doc/html/rfc1624). But unfortunately, some systems have implemented the wrong algorithm. So the strategy of the current network stack for this problem is: when checking, 0xFFFF is considered as the correct checksum; when calculating, 0xFFFF will not be calculated, so there is a special treatment above: when receiving the checksum that should be 0x0000 but is actually 0xFFFF, the function return value is set to true, and the checksum in the packet is set to 0x0000. For UDP, since the checksum of 0 has a special meaning, it is still necessary to change the checksum of 0x0000 to 0xFFFF as specified in the previous section to show the difference.

3. As a supplementary knowledge, when UDP is used as IPv4 Payload, a checksum field of 0 means that the sender does not calculate the checksum, so the receiver ignores the checksum check and considers the checksum correct directly; when the checksum field is not 0, the receiver also needs to verify according to the above check algorithm.

4. The above checksum calculation and checking methods are defined in [RFC 1071](https://datatracker.ietf.org/doc/html/rfc1071).

5. You can use some structures to simplify the parsing process: `struct ip6_hdr`, `struct udphdr` and `struct icmp6_hdr`, and some examples of their use are provided in the code. For UDP/ICMPv6 Length, you can use the `htonl/htons/ntohl/ntohs` functions to convert the byte order.

6. You don't need to handle the input and output, you just need to execute `make grade` locally for local evaluation. In this problem, it is guaranteed that the only data in the packet that may be illegal is checksum.

# Input and output formats

The input file is in PCAP format and contains n IPv6 packets. The main function will use HAL to read the data from the input file and pass it to the function you implement with arguments. You can use Wireshark to open it.

The output file has 2n lines, the 2\*i-1 line is a string Yes/No which indicates whether the checksum of the i IPv6 packet is correct or not; the 2\*i line is a hexadecimal string which indicates the data of the i IPv6 packet after the checksum calculation.
