# lookup

## 题目描述

这一题要求你实现路由表的更新和查询，包括以下四个函数：

```cpp
// 函数注释见代码
void update(bool insert, RoutingTableEntry entry) {
  // TODO
}

// 函数注释见代码
bool prefix_query(in6_addr addr, in6_addr *nexthop, uint32_t *if_index) {
  // TODO
  *nexthop = 0;
  *if_index = 0;
  return false;
}

// 函数注释见代码
int mask_to_len(in6_addr mask) {
  // TODO
  return -1;
}

// 函数注释见代码
in6_addr len_to_mask(int len) {
  // TODO
  return 0;
}
```

其中 `RoutingTableEntry` 的定义见 `router.h` 文件。

你需要实现：

1. 路由表的插入和删除
2. 路由表的查询，返回是否查到，如果查到还需要写入 nexthop 和 if_index

在 `data` 目录中提供了评测使用的数据。

输入数据一共有 $n$ 行，每一行表示一个操作，第一个字符如果是 `I` ，后面跟了四个空格分隔的字符串对应 `addr` `len` `if_index` 和 `nexthop` ，表示插入；第一个字符如果是 `D` ，后面跟了两个空格分隔的字符串对应 `addr` 和 `len` ，表示删除；第一个字符如果是 `Q` ，后面一个 IPv6 地址表示要查询的 IPv6 地址。

输出数据中每一行对应了输入每一行。对于 `I` 或者 `D` 操作，代码会首先判断输入信息中 `addr` 和 `len` 是否合法，如果不合法就输出 `Invalid`，合法则输出 `Valid` 并进行实际的插入或者删除操作。对于输入的 `Q` 操作，则会查询路由表，如果查询到了，输出 `nexthop` 和 `if_index` ，否则输出 `Not Found` 。你不需要在你的代码中处理输入输出。

从这题的数据量来说，实现一个线性查表即可以得到所有分数，我们认为网络课程不是算法课程，不应该在性能上做过高的基本要求。但在后面实际路由器的阶段，线性查表和更优秀的查表算法可能会有性能上的差异。

## 样例 4

见题目目录下的 *lookup_input4.txt* 与 *lookup_answer4.txt*。

## 样例 4 解释

可以看到这个输入数据中，前两行输入构造了这样的一个路由表：

```text
fd00::0102:0300/120 via if9 nexthop fe80::c0a8:0302
fd00::0102:0304/128 via if10 nexthop fe80::c0a8:0901
```

前三次查询分别查到了第二条、第一条、查不到；删除掉路由表第二条以后，前两次查询匹配到第一条，第三次查不到；路由表删光以后，三次都查不到。
