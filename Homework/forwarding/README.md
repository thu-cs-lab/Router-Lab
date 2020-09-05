# forwarding

## 题目描述

这一题要求你进行 IP 的转发逻辑，你需要实现这样的一个函数：

```cpp
// 函数注释见代码
bool forward(unsigned char *packet, unsigned long len) {
  // TODO
  return false;
}

```

你需要实现：

1. 检查 IP 头的校验和是否正确，你可以把第一题的代码放到这里
2. 如果不正确，直接返回 false 不进行下一步处理
3. 如果正确，请把 TTL 减去一，并更新校验和，然后返回 true

在这里你不需要考虑 TTL 为 0 的情况，在最终你的路由器实现中才做要求。

输入数据都已经提供，本题中保证 IP 头里只有 Checksum 可能是不合法的且 TTL 不为 0。

需要注意的是，如果你采用了 Checksum 增量更新的方法，请注意 0x0000 和 0xFFFF 的处理，具体讨论见 RFC 1624: Computation of the Internet Checksum via Incremental Update 。

## 样例 2

见 `data` 目录下的 *forwarding_input2.pcap* 与 *forwarding_output2.ans*。

`forwarding_input2.pcap` 是一个 PCAP 格式的文件，你可以用 Wireshark 软件打开它。

## 样例 2 解释

可以看到这个 PCAP 中一共有 10 个 IP 包，其中第 2 4 6 8 9 个的校验和是错误的，所以输出数据中没有对应的项；其余的 4 个 IP 包都在 TTL 和 Checksum 上按照要求进行了处理。
