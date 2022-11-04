# protocol

## 题目描述

这一题要求你实现 [RFC 2080: RIPng for IPv6](https://datatracker.ietf.org/doc/html/rfc2080) 数据格式的处理，你需要按照 `protocol.cpp` 文件中函数的注释，实现如下两个函数：

```cpp
// 函数注释见代码
RipngErrorCode disassemble(const uint8_t *packet, uint32_t len, RipngPacket *output) {
  // TODO
  return RipngErrorCode::SUCCESS;
}

// 函数注释见代码
uint32_t assemble(const RipngPacket *ripng, uint8_t *buffer) {
  // TODO
  return 0;
}
```

你需要实现：

1. 解析 IPv6、UDP 和 RIPng 的格式，把有用的信息保存下来
2. 如果上一步得到了合法的数据，从保存下来的信息恢复出 RIPng 的传输格式

需要注意的是，这一题中所有数据都是网络传输的数据格式，所以每一个 RIPng 数据字段都是网络字节序，见 `rip.h` 中的注释。

评测所使用的数据都在 `data` 目录下。输入一共有 $n$ 个 IPv6 分组，`main.cpp` 会调用你的代码来判断每个分组是不是一个合法的 RIPng 报文，如果是，则保存下来，输出一行 `Valid {numEntries} {command}`，之后是 `numEntries` 行，每一行的输出分别对应 `prefix_or_nh` `route_tag` `prefix_len` 和 `metric`，再输出一行，是重新构造出来的 RIPng 报文，用十六进制格式输出；如果不合法，就输出不合法的地方。

## 样例 1

见题目目录下的 *protocol_input1.pcap* 与 *protocol_answer1.txt*。

## 样例 1 解释

用 Wireshark 打开，可以看到有两个合法的 RIPng packet，它们的内容如下：

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

可以看到都是合法的 RIPng packet。把对应的内容填入结构体即可，需要注意字节序。
