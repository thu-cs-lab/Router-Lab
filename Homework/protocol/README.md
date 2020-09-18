# protocol

## 题目描述

这一题要求你实现 [RFC 2453: RIP Version 2](https://tools.ietf.org/html/rfc2453) 数据格式的处理，你需要按照 `protocol.cpp` 文件中函数的注释，实现如下两个函数：

```cpp
// 函数注释见代码
bool disassemble(const uint8_t *packet, uint32_t len, RipPacket *output) {
  // TODO
  return false;
}

// 函数注释见代码
uint32_t assemble(const RipPacket *rip, uint8_t *buffer) {
  // TODO
  return 0;
}
```

你需要实现：

1. 解析 IP 、 UDP 和 RIPv2 的格式，把有用的信息保存下来
2. 如果上一步得到了合法的数据，从保存下来的信息恢复出 RIP 的传输格式

需要注意的是，这一题中所有数据都是网络传输的数据格式，所以每一个 RIP 数据字段都是网络字节序，见 `rip.h` 中的注释。

评测所使用的数据都在 `data` 目录下。输入一共有 $n$ 个包，`main.cpp` 会调用你的代码来判断每个包是不是一个合法的 RIP 包，如果是，则保存下来，输出一行 `Valid {numEntries} {command}`，之后是 `numEntries` 行，每一行四个十六进制数，分别对应 `addr` `mask` `nexthop` 和 `metric` ，再输出一行，是重新构造出来的 RIP 包，用十六进制格式输出；如果不合法，就输出 `Invalid` 。

## 样例 1

见题目目录下的 *protocol_input1.pcap* 与 *protocol_output1.out*。

## 样例 1 解释

用 Wireshark 打开，可以看到有两个合法的 RIP 包，它们的内容是一样的：

```text
Routing Information Protocol
    Command: Response (2)
    Version: RIPv2 (2)
    IP Address: 192.168.5.0, Metric: 1
        Address Family: IP (2)
        Route Tag: 0
        IP Address: 192.168.5.0
        Netmask: 255.255.255.0
        Next Hop: 0.0.0.0
        Metric: 1
```

把对应的内容填入结构体即可，需要注意字节序。
