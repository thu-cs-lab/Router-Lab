# lookup

## 题目描述

这一题要求你实现路由表的更新和查询，包括以下两个函数：

```cpp
// 函数注释见代码
void update(bool insert, RoutingTableEntry entry) {
  // TODO:
}

// 函数注释见代码
bool query(uint32_t addr, uint32_t *nexthop, uint32_t *if_index) {
  // TODO:
  *nexthop = 0;
  *if_index = 0;
  return false;
}
```

其中 `RoutingTableEntry` 的定义见 `router.h` 文件。

你需要实现：

1. 路由表的插入和删除
2. 路由表的查询，返回是否查到，如果查到还需要写入 nexthop 和 if_index

请一定注意，题中的 addr 和 nexthop 都是网络字节序存储的，你可以使用位运算或者 ntohl 函数进行字节序的转换。

在 `data` 目录中提供了评测使用的数据。输入数据一共有 $n$ 行，每一行表示一个操作，第一个字符如果是 `I` ，后面跟了四个数字对应 `addr` `len` `if_index` 和 `nexthop` ，表示插入；第一个字符如果是 `D` ，后面跟了两个数字对应 `addr` 和 `len` ，表示删除；第一个字符如果是 `Q` ，后面一个数字表示要查询的 IPv4 地址。输出中每一行对应了输入中 `Q` 操作的行，如果查询到了，则输出 `nexthop` 和 `if_index` ，否则输出 `Not Found` 。你不需要在你的代码中处理输入输出。

从这题的数据量来说，实现一个线性查表即可以得到所有分数，我们认为网络课程不是算法课程，不应该在性能上做过高的基本要求，如果你确实实现了很优秀的算法，请到 [评测鸭的《测测你的路由器》题目](https://duck.ac/problem/router32) 中评测。但在后面实际路由器的阶段，线性查表和更优秀的查表算法可能会有性能上的差异。

## 样例 4

见题目目录下的 *lookup_input4.in* 与 *lookup_output4.out*。

## 样例 4 解释

需要注意，对于文件中的数字， 0x04030201 代表 1.2.3.4 ，以此类推。可以看到这个输入数据中构造了这样的一个路由表：

```text
1.2.3.0 netmask 255.255.255.0 via if9 nexthop 192.168.3.2
1.2.3.4 netmask 255.255.255.255 via if10 nexthop 192.168.9.1
```

前三次查询分别查到了第二条、第一条、查不到；删除掉路由表第二条以后，两次查询匹配到第一条，第三次查不到；路由表删光以后，三次都查不到。

