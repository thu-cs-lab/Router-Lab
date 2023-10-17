# protocol

## 题目描述

这一题要求你实现 [RFC 5340: OSPF for IPv6](https://datatracker.ietf.org/doc/html/rfc5340/) 数据格式的处理，你需要按照 `protocol_ospf.cpp` 文件中函数的注释，实现如下两个函数：

```cpp
// 函数注释见代码
OspfErrorCode parse_ip(const uint8_t *packet, uint32_t len, const uint8_t **lsa_start, int *lsa_num) {
  // TODO
  return OspfErrorCode::SUCCESS;
}

// 函数注释见代码
OspfErrorCode disassemble(const uint8_t* lsa, uint16_t buf_len, uint16_t *len, RouterLsa *output) {
  // TODO
  return OspfErrorCode::SUCCESS;
}
```

你需要实现：

1. 解析 IPv6、OSPF 和 LSU（Link State Update）报文的格式，获取 LSA（Link State Advertisement）的起始位置和数量
2. 如果上一步得到了合法的数据，解析 LSA

需要注意的是，这一题中所有数据都是网络传输的数据格式，所以每一个 OSPF 数据字段都是网络字节序。

评测所使用的数据都在 `data` 目录下。输入一共有 $n$ 个 IPv6 分组，`main.cpp` 会调用你的代码来判断每个分组是不是一个合法的 OSPF 报文。如果合法，则保存下来，输出一行 `Success`。之后是 `lsa_num` 组，每一组先输出 LSA 类型和相应的 LSA 长度。如果 LSA 是 Router-LSA，就进一步输出其 `ls_age` `link_state_id` `advertising_router` 和 `ls_sequence_number`。如果 Router-LSA 含有一些 Entry，则对于每个 Entry，先输出一行 `Entry`，再进一步输出对应的 `type` `metric` `interface_id` `neighbor_interface_id` `neighbor_router_id`。如果不合法，就输出不合法的地方。

## 样例 1

见题目目录下的 *protocol_ospf_input1.pcap* 与 *protocol_ospf_answer1.txt*。

## 样例 1 解释

用 Wireshark 打开，可以看到有一个合法的 OSPF 报文，且是一个 LSU 报文。这个 LSU 报文包括了 10 个 LSA，其中有两个是 Router-LSA。由此，只需要将这两个 Router-LSA 中的信息填入对应的结构体即可。
