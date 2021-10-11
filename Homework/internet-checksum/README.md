# internet-checksum

## 题目描述

在 IPv6 中，Header 不再有校验和（checksum）字段，而在 UDP 和 ICMPv6 协议中计算校验和的时候，需要将 IPv6 Header 的一部分数据考虑进来，这就是 Pseudo Header。

你需要在 `checksum.cpp` 中实现下面的函数 `validateAndFillChecksum`，这个函数接收一个 IPv6 的 packet，在 IPv6 Header 之后一定是一个 UDP 或者 ICMPv6 的 packet。该函数的返回值为 UDP 或者 ICMPv6 中的校验和是否正确；同时，无论原来的校验和是否正确，该函数返回时，packet 中的校验和应该被填充为正确的值。

```cpp
// 函数注释见代码
bool validateAndFillChecksum(uint8_t *packet, size_t len) {
  return true;
}
```

校验和的计算方式如下：

1. 将校验和字段填充为 0
2. 将 IPv6 Pseudo Header 拼接上 UDP/ICMPv6 packet 视作大端序 16 位整数的数组，将所有 16 位整数相加
3. 如果和发生溢出，则将其截断为低 16 位及溢出部分，然后将溢出部分加到低 16 位
   (如 0x1CB2F -> 0xCB2F + 0x1)
4. 如果上述操作中又发生了溢出，则重复上述操作，直到不发生溢出
5. 将得到的结果按位取反，使用大端序填充到校验和字段

检验校验和是否正确的方式如下：

1. 将 IPv6 Pseudo Header 拼接上 UDP/ICMPv6 packet 视作大端序 16 位整数的数组，将所有 16 位整数相加
2. 如果和发生溢出，则将其截断为低 16 位及溢出部分，然后将溢出部分加到低 16 位
   (如 0x1CB2F -> 0xCB2F + 0x1)
3. 如果上述操作中又发生了溢出，则重复上述操作，直到不发生溢出
4. 判断结果是否等于 0xFFFF

以上的计算方式和检验方式定义在 [RFC 1071](https://datatracker.ietf.org/doc/html/rfc1071) 中。需要注意数据长度是奇数的情况下的处理。

以上的算法有这样一个性质，即按照计算的算法得到的校验和，检验得到的一定是正确的结果。但反过来其实不然，我们可以观察到这么一个事情，即校验和计算方式得到的结果不可能是 0xFFFF：因为从第 2 步到第 4 步的结果运算过程中，求和的结果不可能是 0x0000，因此在第 5 步取反以后，最终的校验和不可能是 0xFFFF。但如果校验和等于 0xFFFF，按照检验校验和正确的算法，得到的结果依然是正确的。你可以想一想，为什么在这种运算方式下，将 0x0000 替换为 0xFFFF 或者反过来把 0xFFFF 替换为 0x0000 都不影响求和的计算结果。

同学你可能会问，如果大家都用上面的计算方法得到校验和，为何还会出现 0xFFFF 的情况呢？在 [RFC 1141](https://datatracker.ietf.org/doc/html/rfc1141) 中定义的增量更新的算法因为设计上的纰漏，会导致计算出 0xFFFF 的校验和，在之后的 [RFC 1624](https://datatracker.ietf.org/doc/html/rfc1624) 中才得以修复。但很可惜的是，一些系统已经实现了错误的算法，因此目前的网络栈对这个问题的策略是：校验的时候，认为 0xFFFF 是正确的校验和；计算的时候，不会计算出 0xFFFF。在本题中，当接受到校验和应该为 0x0000 但实际为 0xFFFF 的情况时，函数返回值设为 true，并将 packet 中的校验和设为 0x0000。

特别地，在 UDP 中，校验和字段为 0 表示没有进行校验和的计算。然而，由于 IPv6 头部中没有校验和字段等原因，承载于 IPv6 的 UDP 要求必须进行校验和计算，即校验和字段不为 0。此时，接收方在检查时应当认为校验和字段为 0 是错误的。对于发送方，如果计算出的校验和为 0，则需要设置校验和字段为 0xFFFF。作为补充知识，当 UDP 作为 IPv4 的 Payload 时，校验和字段为 0 表示发送方没有进行校验和的计算，此时接收方忽略校验和检查，直接认为校验和是正确的；当校验和字段不为 0 时，接收方同样需要按照上述检验算法进行验证。

IPv6 Pseudo Header 由下面几个东西组成：

1. 16 字节的 Source IPv6 Address
2. 16 字节的 Destination IPv6 Address
3. 4 字节的 UDP/ICMPv6 Length（网络字节序）
4. 3 字节的 0，然后是 1 字节的 Next Protocol（对 UDP 来说是 17，对 ICMPv6 来说是 58）

可以对照 [UDP Checksum](https://en.wikipedia.org/wiki/User_Datagram_Protocol#IPv6_pseudo_header)  和 [ICMPv6 Checksum](https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol_for_IPv6#Checksum) 网页上的表格进行实现。

提示：你可以用一些结构体来简化解析过程：`struct ip6_hdr`、`struct udphdr` 和 `struct icmp6_hdr`，代码中已经提供了一些使用的例子。对于 UDP/ICMPv6 Length，你可以用 `htonl/htons/ntohl/ntohs` 函数来转换字节序。

你不需要处理输入输出，你只需要在本地执行 `make grade` 就可以进行本地评测。在本题中，保证 packet 中的数据只有 checksum 可能是不合法的。

# 输入输出格式

输入文件是 PCAP 格式，里面含有 n 个 IPv6 packet，main 函数会使用 HAL 读取输入文件中的数据，并通过参数传递给你实现的函数。你可以用 Wireshark 去打开它。

输出文件有 2n 行，第 2\*i-1 行一个字符串 Yes/No 表示第 i 个 IPv6 packet 的校验和验证是否正确；第 2\*i 行一个十六进制字符串表示第 i 个 IPv6 packet 进行校验和计算后的数据。
