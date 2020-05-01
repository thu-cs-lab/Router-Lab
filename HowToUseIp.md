## 附录：`ip` 命令的使用

在本文中几次提到了 `ip` 命令的使用，它的全名为 iproute2，是当前管理 Linux 操作系统网络最常用的命令之一。需要注意的是，涉及到更改的命令都需要 root 权限，所以需要在命令前加一个 `sudo` （注意空格）表示用 root 权限运行。

第一个比较重要的子命令是 `ip a`，它是 `ip addr` 的简写，意思是列出所有网口信息和地址信息，如：

```text
2: enp14s0: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN group default qlen 1000
    link/ether 12:34:56:78:9a:bc brd ff:ff:ff:ff:ff:ff
```

代表有一个名为 `enp14s0` 的网口，状态为 `DOWN` 表示没有启用，反之 `UP` 表示已经启用，下面则是它的 MAC 地址，其他部分可以忽略。

每个网口可以有自己的 IP 地址，如：

```text
2: enp14s0: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state UP group default qlen 1000
    link/ether 12:34:56:78:9a:bc brd ff:ff:ff:ff:ff:ff
    inet 1.2.3.4/26 brd 1.2.3.127 scope global dynamic enp14s0
       valid_lft 233sec preferred_lft 233sec
    inet6 2402:f000::233/128 scope global dynamic noprefixroute
       valid_lft 23333sec preferred_lft 23333sec
```

这代表它有一个 IPv4 地址 1.2.3.4 和一个 IPv6 地址 2404:f000::233，其表示方法为 CIDR，即地址和前缀长度。

如果要给网口配置一个 IP 地址，命令为 `ip addr add $addr/$prefix_len dev $interface` 其中 $addr $prefix_len $interface 是需要你填入的，比如对于上面这个例子，可能之前执行过 `ip addr add 1.2.3.4/26 dev enp14s0` 。删除只要把 add 换成 del 即可。

需要注意的是，在运行你实现的路由器的时候，请不要在相关的网口上配置 IP 地址，因为 HAL 绕过了 Linux 网络栈，如果你配置了 IP 地址，在 Linux 和路由器的双重作用下可能有意外的效果。

第二个重要的子命令是 `ip l`，是 `ip link` 的简写，一般直接使用 `ip link set $interface up` 和 `ip link set $interface down` 来让一个 `DOWN` 的网口变成 `UP`，也可以反过来让一个 `UP` 的网口变成 `DOWN`。注意，在一些情况下（例如网线没插等等），它可能会失败。

第三个重要的子命令是 `ip r`，是 `ip route` 的简写，它会显示 Linux 系统中的路由表：

```text
default via 1.2.3.1 dev enp14s0 proto static
1.2.3.0/26 dev enp14s0 proto kernel scope link src 1.2.3.4
```

我们也在上文中数次用了类似的语法表示一个路由表。每一项的格式如下：

```text
ip/prefix dev interface scope link 表示在这个子网中，所有的 IP 都是直连可达
ip/prefix via another_ip dev interface 表示去往目标子网的 IP 包，下一跳都是 another_ip ，通过 interface 出去
default via another_ip dev interface 这里 default 代表 0.0.0.0/0 ，其实是上一种格式
```

至于一些额外的类似 `proto static` `proto kernel` `src 1.2.3.4` 的内容可以直接忽略。

如果要修改的话，可以用 `ip route add` 接上你要添加的表项，相应地 `ip route del` 就是删除。如果要删掉上面的默认路由，可以用 `ip route del default via 1.2.3.1 dev enp14s0` 实现。