## 附录：出现问题如何调试

对于实验的第二阶段，验收测试里第一项就是 ICMP 的连通性测试，一个 ICMP Echo Request 经过 PC1 - R1 - R2 - R3 - PC2 ，再一个对应的 ICMP Echo Reply 经过 PC2 - R3 - R2 - R1 - PC1，这么多步骤错了一个都会导致不能连通，下面介绍一个调试的思路：

1. 在 PC1 上一直开着 ping 到 PC2
2. 从 PC1 开始，一跳一跳地抓包，即 pc1r1，r1r2，r2r3，r3pc2 依次抓包，找到第一次没有出现 Echo Request 的地方，比如 pc1r1 和 r1r2 可以抓到，而 r2r3 抓不到，那说明问题可能在 r2；如果一直到 PC2 都能抓到 Echo Request，就反过来抓 Echo Reply，一样可以定位到出问题的点。
3. 找到可能出问题的点后，首先检查进入这个点的包的格式对不对，比如 IP 头的 checksum 和 length 是否正确（打开 Wireshark 的 IP Header Checksum 检查）
4. 如果进入的包格式是错的，那出问题的点是这个点的前一个，一般来说前一个就是 R2，这种情况一般是转发的时候 Checksum 更新有问题；如果前一个不是 R2，并且你采用了 netns ，请在上面的 FAQ 找到相应的问题
5. 如果进入的包格式是对的，如果这个点是：
   1. R2：检查你的路由表和路由表的查询结果
   2. 其他：用 `ip r` 命令看有没有 ICMP 目的地址应该匹配上的正确路由，如果没有那就是 RIP 协议还有问题
      1. 如果怀疑 RIP 协议有问题，那就在 r1r2 和 r2r3 上抓包，着重注意源地址是 192.168.3.2 和 192.168.4.1 （即 R2 发出的）的 RIP Response 包，检查以下几点：
         1. IP 头和 UDP 头的长度和下面的 RIP Entry 数量要匹配
         2. IP Header Checksum 和 UDP Checksum
         3. RIP 的 Version 为 2
         4. RIP Entry 数量需要在 [1,25] 的范围内
         5. 对于每条 RIP Entry：
            1. Address Family = 2
            2. Route Tag = 0
            3. Mask 的二进制要么全是 1，要么全是 0，要么是连续的 1 接着连续的 0
            4. IP Address & ~Mask == 0
            5. Nexthop 要么为 0 ，要么和 IP 源地址在同一个网段
            6. Metric 在 [1,16] 的范围内