# Router-Lab

最后更新：2020/05/01 14:40

![C/C++ CI](https://github.com/z4yx/Router-Lab/workflows/C/C++%20CI/badge.svg)

## 版权声明

本项目为清华大学 2020-2021 年秋季学期计算机系《计算机网络原理》课程实验。
**未经作者授权，禁止用作任何其他用途，包括且不限于在其他课程或者其他学校中使用。**
作者保留一切追究侵权责任的权利。

## 简介

<details>
    <summary> 目录 </summary>

* [如何使用框架](#如何使用框架)
* [如何进行本地自测](#如何进行本地自测)
* [如何进行在线测试（暗号：框）](#如何进行在线测试暗号框)
* [实验验收的流程](#实验验收的流程)
* [建议的实验思路](#建议的实验思路)
  * [如何启动并配置一个比较标准的 RIP 实现](#如何启动并配置一个比较标准的-rip-实现)
  * [如何在一台计算机上进行真实测试](#如何在一台计算机上进行真实测试)
* [FAQ（暗号：档）](#faq暗号档)
* [附录：ip 命令的使用](#附录ip-命令的使用)
* [附录：树莓派系统的配置和使用](#附录树莓派系统的配置和使用)
* [附录： make 命令的使用和 Makefile 的编写](#附录-make-命令的使用和-makefile-的编写)
* [附录：出现问题如何调试](#附录出现问题如何调试)
* [名词解释](#名词解释)
* [项目作者](#项目作者)

</details>

这里是 2020 年网络原理课程原理课程实验采用的框架。它有以下的设计目标：

1. 降低难度，把与底层打交道的部分抽象成通用的接口，减少学习底层 API 的负担
2. 复用代码，在一个平台上编写的程序可以直接用于其他平台
3. 方便测试，提供文件读写 PCAP 的后端，可以直接用数据进行黑箱测试

实验的目标是完成一个具有以下功能的路由器：

1. 转发：收到一个路过的 IP 包，通过查表得到下一棒（特别地，如果目的地址直接可达，下一棒就是目的地址），然后“接力”给下一棒。
2. 路由：通过 RIP 协议，学习到网络的拓扑信息用于转发，使得网络可以连通。

如果你参加的是联合实验，请阅读 `Router-Lab-Joint` 仓库下的 README。

请仔细阅读下面的文本，在问问题时，请把以下每个括号中的 **暗号** 按顺序连在一起重复一遍然后再说你的问题。

本文档默认你已经在软件工程、编译原理、程序设计训练等课程中已经学习到了足够的 Git 、Make 、SSH 、Python3 和 Linux 的使用知识。如果你用的是 Windows 系统，你可以 WSL/虚拟机/下发的树莓派上进行以下所有相关的操作。无论你电脑是用的是什么系统，最终都需要在树莓派的 Linux 上运行路由器。

如果你运行的是 Debian 系列发行版（包括 Ubuntu、Raspbian），你可以一把梭地安装所有可能需要的依赖：

```bash
sudo apt install git make cmake python3 python3-pip libpcap-dev libreadline-dev libncurses-dev wireshark tshark iproute2 g++
pip3 install pyshark
```

其他发行版也有类似的包管理器安装方法。

## 如何使用框架

[如何使用框架](./HowToUse.md)

## 如何进行本地自测

在 `Homework` 目录下提供了若干个题目，通过数据测试你的路由器中核心功能的实现。你需要在被标记 TODO 的函数中补全它的功能，通过测试后，就可以更容易地完成后续的实践。

有这些目录：

```text
checksum： 计算校验和
forwarding： 转发逻辑
lookup： 路由表查询和更新
protocol： RIP 协议解析和封装
boilerplate： 用以上代码实现一个路由器
```

每个题目都有类似的结构（以 `checksum` 为例）：

```text
data： 数据所在的目录
checksum.cpp：你需要修改的地方
grade.py：一个简单的评分脚本，它会编译并运行你的代码，进行评测
main.cpp：用于评测的交互库，你不需要修改它
Makefile：用于编译并链接 HAL 、交互库和你实现的代码
```

使用方法（在 Homework/checksum 目录下执行）：

```bash
pip install pyshark # 仅第一次，一些平台下要用 pip3 install pyshark
# 修改 checksum.cpp（暗号：读）
make # 编译，得到可以执行的 checksum
./checksum < data/checksum_input1.pcap # 你可以手动运行来看效果
make grade # 也可以运行评分脚本，实际上就是运行python3 grade.py
```

它会对每组数据运行你的程序，然后比对输出。如果输出与预期不一致，它会把出错的那一个数据以 Wireshark 的类似格式打印出来，并且用 diff 工具把你的输出和答案输出的不同显示出来。

这里很多输入数据的格式是 PCAP ，它是一种常见的保存网络流量的格式，它可以用 Wireshark 软件打开来查看它的内容，也可以自己按照这个格式造新的数据。需要注意的是，为了区分一个以太网帧到底来自哪个虚拟的网口，我们所有的 PCAP 输入都有一个额外的 VLAN 头，VLAN 0-3 分别对应虚拟的 0-3 ，虽然实际情况下不应该用 VLAN 0，但简单起见就直接映射了。（暗号：了）

## 如何进行在线测试（暗号：框）

选课的同学还需要在 OJ 上进行你的代码的提交，它会进行和你本地一样的测试，数据也基本一致。你提交的代码会用于判断你掌握的程度和代码查重。

一般来说，你只需要把你修改的函数的整个文件提交到对应题目即可，如 `Homework/checksum/checksum.cpp` 提交到 `checksum` 题目中。如果通过了测试，你实现的这个函数之后就可以继续用在你的路由器的实现之中。

需要注意的是，测试采用的数据并不会面面俱到，为了减少在真实硬件（如树莓派、FPGA）上调试的困难，建议同学们自行设计测试样例，这样最终成功的可能性会更高。

## 实验验收流程

[实验验收流程](./HowToCheck.md)

## 建议的实验思路

推荐的实验流程是：（暗号：架）

1. 克隆本仓库，认真阅读文档
2. 运行 Example 下面的程序，保证自己环境正确配置了
3. 进行 Homework 的编写，编写几个关键的比较复杂容易出错的函数，保证这些实现是正确的
4. 把上一步实现的几个函数和 HAL 配合使用，实现一个真实可用的路由器

建议采用的一些调试工具和方法：（暗号：文）

1. Wireshark：无论是抓包还是查看评测用到的所有数据的格式，都是非常有用的，一定要学会
2. 编写测试的输入输出，这个仓库的 `Datagen` 目录下有一个用 Rust 编写的 PCAP 测试样例生成程序，你可以修改它以得到更适合你的代码的测试样例，利用 Wireshark 确认你构造的样例确实是合法的
3. 运行一些成熟的软件，然后抓包看它们的输出是怎样的，特别是调试 RIP 协议的时候，可以自己用 BIRD（BIRD Internet Routing Daemon）跑 RIP 协议然后抓包，有条件的同学也可以自己找一台企业级的路由器进行配置（选计算机网络专题训练体验一下），当你的程序写好了也可以让你的路由器和它进行互通测试。当大家都和标准实现兼容的时候，大家之间兼容的机会就更高了。

关于第四步，一个可能的大概的框架如下：

```cpp
int main() {
    // 0a. 初始化 HAL，打开调试信息
    HAL_Init(1, addrs);
    // 0b. 创建若干条 /24 直连路由
    for (int i = 0; i < N_IFACE_ON_BOARD;i++) {
        RoutingTableEntry entry = {
            .addr = addrs[i] & 0x00FFFFFF, // big endian
            .len = 24,
            .if_index = i,
            .nexthop = 0 // means direct
        };
        update(true, entry);
    }

    uint64_t last_time = 0;
    while (1) {
        // 获取当前时间，处理定时任务
        uint64_t time = HAL_GetTicks();
        if (time > last_time + 30 * 1000) {
            // 每 30s 做什么
            // 例如：超时？发 RIP Request/Response？
            last_time = time;
        }

        // 轮询
        int mask = (1 << N_IFACE_ON_BOARD) - 1;
        macaddr_t src_mac;
        macaddr_t dst_mac;
        int if_index;
        int res = HAL_ReceiveIPPacket(mask, packet, sizeof(packet), src_mac,
                                    dst_mac, 1000, &if_index); // 超时为 1s
        if (res > 0) {
            // 1. 检查是否是合法的 IP 包，可以用你编写的 validateIPChecksum 函数，还需要一些额外的检查
            // 2. 检查目的地址，如果是路由器自己的 IP（或者是 RIP 的组播地址），进入 3a；否则进入 3b
            // 3a.1 检查是否是合法的 RIP 包，可以用你编写的 disassemble 函数检查并从中提取出数据
            // 3a.2 如果是 Response 包，就调用你编写的 query 和 update 函数进行查询和更新，
            //      注意此时的 RoutingTableEntry 可能要添加新的字段（如metric、timestamp），
            //      如果有路由更新的情况，可能需要构造出 RipPacket 结构体，调用你编写的 assemble 函数，
            //      再把 IP 和 UDP 头补充在前面，通过 HAL_SendIPPacket 把它发到别的网口上
            // 3a.3 如果是 Request 包，就遍历本地的路由表，构造出一个 RipPacket 结构体，
            //      然后调用你编写的 assemble 函数，另外再把 IP 和 UDP 头补充在前面，
            //      通过 HAL_SendIPPacket 发回询问的网口
            // 3b.1 此时目的 IP 地址不是路由器本身，则调用你编写的 query 函数查询，
            //      如果查到目的地址，如果是直连路由， nexthop 改为目的 IP 地址，
            //      用 HAL_ArpGetMacAddress 获取 nexthop 的 MAC 地址，如果找到了，
            //      就调用你编写的 forward 函数进行 TTL 和 Checksum 的更新，
            //      通过 HAL_SendIPPacket 发到指定的网口，
            //      在 TTL 减到 0 的时候建议构造一个 ICMP Time Exceeded 返回给发送者；
            //      如果没查到目的地址的路由，建议返回一个 ICMP Destination Network Unreachable；
            //      如果没查到下一跳的 MAC 地址，HAL 会自动发出 ARP 请求，在对方回复后，下次转发时就知道了
        } else if (res == 0) {
            // Timeout, ignore
        } else {
            fprintf(stderr, "Error: %d\n", res);
            break;
        }
    }
    return 0;
}
```

你可以直接基于 `Homework/boilerplate` 下的代码，把上面的代码实现完全。代码中在发送 RIP 包的时候，会涉及到 IP 头的构造，由于不需要用各种高级特性，可以这么设定：V=4，IHL=5，TOS(DSCP/ECN)=0，ID=0，FLAGS/OFF=0，TTL=1，其余按照要求实现即可。

### 如何启动并配置一个比较标准的 RIP 实现

你可以用一台 Linux 机器，连接到你的路由器的一个网口上，一边抓包一边运行一个 RIP 的实现。我们提供一个 BIRD（BIRD Internet Routing Daemon，安装方法 `apt install bird`）的参考配置，以 Debian 为例，如下修改文件 `/etc/bird.conf` 即可。

<details>
    <summary> BIRD v2.0 配置 </summary>

```conf
# log "bird.log" all; # 可以将 log 输出到文件中
# debug protocols all; # 如果要更详细的信息，可以打开这个

router id 网口IP地址; # 随便写一个，保证唯一性即可

protocol device {
}

protocol kernel {
    # 表示 BIRD 会把系统的路由表通过 RIP 发出去，也会把收到的 RIP 信息写入系统路由表
    # 你可以用 `ip route` 命令查看系统的路由表
    # 退出 BIRD 后从系统中删除路由
    persist no;
    # 从系统学习路由
    learn;
    ipv4 {
        # 导出路由到系统，可以用 `ip r` 看到
        # 也可以用 `export none` 表示不导出，用 birdc show route 查看路由
        export all;
    };
}

protocol static {
    ipv4 { };
    route 1.2.3.4/32 via "网口名称"; # 可以手动添加一个静态路由方便调试，只有在这个网口存在并且为 UP 时才生效
}

protocol rip {
    ipv4 {
        import all;
        export all;
    };
    debug all;
    interface "网口名称" { # 网口名称必须存在，否则 BIRD 会直接退出
        version 2;
        update time 5; # 5秒一次更新，方便调试
    };
}
```

</details>

<details>
    <summary> BIRD v1.6 配置 </summary>

```conf
# log "bird.log" all; # 可以将 log 输出到文件中
# debug protocols all; # 如果要更详细的信息，可以打开这个

router id 网口IP地址; # 随便写一个，保证唯一性即可

protocol device {
}

protocol kernel {
    # 表示 BIRD 会把系统的路由表通过 RIP 发出去，也会把收到的 RIP 信息写入系统路由表
    # 你可以用 `ip route` 命令查看系统的路由表
    # 退出 BIRD 后从系统中删除路由
    persist off;
    # 从系统学习路由
    learn;
    # 导出路由到系统，可以用 `ip r` 看到
    # 也可以用 `export none` 表示不导出，用 birdc show route 查看路由
    export all;
}

protocol static {
    route 1.2.3.4/32 via "网口名称"; # 可以手动添加一个静态路由方便调试，只有在这个网口存在并且为 UP 时才生效
}

protocol rip {
    import all;
    export all;
    debug all;
    interface "网口名称" { # 网口名称必须存在，否则 BIRD 会直接退出
        version 2;
        update time 5; # 5秒一次更新，方便调试
    };
}
```

</details>

这里的网口名字对应你连接到路由器的网口，也要配置一个固定的 IP 地址，需要和路由器对应网口的 IP 在同一个网段内。配置固定 IP 地址的命令格式为 `ip a add IP地址/前缀长度 dev 网口名称`，你可以用 `ip a` 命令看到所有网口的信息。

启动服务（如 `systemctl start bird`）后，你就可以开始抓包，同时查看 bird 打出的信息（`journalctl -f -u bird`），这对调试你的路由器实现很有帮助。

你也可以直接运行 BIRD（`bird -c /etc/bird.conf`），可在命令选项中加上 `-d` 把程序放到前台，方便直接退出进程。若想同时开多个 BIRD，则需要给每个进程指定单独的 PID 文件和 socket，如 `bird -d -c bird1.conf -P bird1.pid -s bird1.socket` 。

在安装 BIRD（`sudo apt install bird`）之后，它默认是已经启动并且开机自启动。如果要启动 BIRD，运行 `sudo systemctl start bird`；停止 BIRD： `sudo systemctl stop bird`；重启 BIRD：`sudo systemctl restart bird`；打开开机自启动：`sudo systemctl enable bird`；关闭开机自启动：`sudo systemctl disable bird`。

配合 BIRD 使用的还有它的客户端程序 `birdc`，它可以连接到 BIRD 服务并且控制它的行为。默认情况下 birdc 会连接系统服务（systemctl 启动）的 BIRD，如果启动 BIRD 时采用了 `-s` 参数，那么 birdc 也要指定同样的 socket 路径。

对于一条静态路由（如 `route 1.2.3.0/24 via "abcd"`），它只有在 `abcd` 处于 UP 状态时才会生效，如果你只是想让 BIRD 向外宣告这一条路由，可以用 `lo` （本地环回）代替 `abcd` 并且运行 `ip l set lo up`。你可以用 `birdc show route` 来确认这件事情。

### 如何在一台计算机上进行真实测试

为了方便测试，你可以在一台计算机上模拟上述验收环境的网络拓扑，并相应在模拟出的五台“主机”中运行不同的程序（如 BIRD / 你实现的路由器软件 / ping 等客户端工具）。这对于你的调试将有很大的帮助。我们建议你采用下列的两种方式：

1. 使用虚拟机安装多个不同的操作系统，并将它们的网络按照需要的拓扑连接。这一方法思路简单，并且可以做到与真实多机环境完全相同，但可能消耗较多的资源。
2. 使用 Linux 提供的 network namespace 功能，在同一个系统上创建多个相互隔离的网络环境，并使用 veth （每对 veth 有两个接口，可以处在不同的 namespace 中，可以理解为一条虚拟的网线）将它们恰当地连接起来。这一方法资源占用少，但是对 Linux 使用经验和网络配置知识有较高的需求。我们在下面提供了一些简单的指导：

和 network namespace 相关的操作的命令是 `ip netns`。例如我们想要创建两个 namespace 并让其能够互相通信：

```bash
ip netns add net0 # 创建名为 "net0" 的 namespace
ip netns add net1
ip link add veth-net0 type veth peer name veth-net1 # 创建一对相互连接的 veth pair
ip link set veth-net0 netns net0 # 将 veth 一侧加入到一个 namespace 中
ip link set veth-net1 netns net1 # 配置 veth 另一侧
ip netns exec net0 ip link set veth-net0 up
ip netns exec net0 ip addr add 10.1.1.1/24 dev veth-net0 # 给 veth 一侧配上 ip 地址
ip netns exec net1 ip link set veth-net1 up
ip netns exec net1 ip addr add 10.1.1.2/24 dev veth-net1
```

配置完成后你可以运行 `ip netns exec net0 ping 10.1.1.2` 来测试在 net0 上是否能够 ping 到 net1。

你还可以运行 `ip netns exec net0 [command]` 来执行任何你想在特定 namespace 下执行的命令，也可以运行 `ip netns exec net0 bash` 打开一个网络环境为 net0 的 bash。

如果你在一个 netns 中用 Linux 自带的功能做转发（例如 R1 和 R3），需要运行如下命令（root 身份，重启后失效）：

```shell
echo 1 > /proc/sys/net/ipv4/conf/all/forwarding
```

上面的 all 可以替换为 interface 的名字。在用这种方法的时候需要小心 Linux 自带的转发和你编写的转发的冲突，在 R2 上不要用 `ip a` 命令配置 IP 地址。

## FAQ（暗号：档）

Q：暗号是干嘛的，为啥要搞这一出？

A：总是有同学不认真阅读文档，所以，如果你阅读到了这里，请心里默念暗号：_______

Q：我用的是纯命令行环境，没有 Wireshark 图形界面可以用，咋办？

A：你可以用 tcpdump 代替 Wireshark，它的特点是一次性输出所有内容；或者用 tshark，是 Wireshark 官方的 CLI 版本；也可以用 termshark ，它是 Wireshark 的 TUI 版，操作方式和 Wireshark 是一致的。比较常用的 tshark 用法是 `sudo tshark -i [interface_name] -V -l [filter]` ，其中 `interface_name` 是网卡名字，如 `eth0` ，`-V` 表示打印出解析树， `-l` 表示关闭输出缓冲， `[filter]` 表示过滤，常见的有 `arp` `ip` `icmp` 等等。

Q: 运行 `grade.py` 的时候，提示找不到 tshark ，怎么办？

A: 用你的包管理器安装 wireshark 或者 tshark 都行。如果你在使用 Windows，需要注意 Windows 版的 Wireshark 和 WSL 内部的 Wireshark 是需要分别安装的。

Q: tshark 好像默认不会检查 IP Header Checksum 等各种 Checksum，我怎么让它进行校验？

A: 给它命令行参数 `-o ip.check_checksum:TRUE` `-o tcp.check_checksum:TRUE` 和 `-o udp.check_checksum:TRUE` 就可以打开它的校验功能。如果你使用 Wireshark，直接在 Protocol Preferences 中选择即可。

Q：为啥要搞 HAL 啊，去年让大家用 Linux 的 Raw Socket ，不也有人搞出来了吗？

A：我们认为去年的 Linux 的 Raw Socket 是比较古老而且需要同学编写很多人冗余代码的一套 API，另外比较复杂的 Quagga 的交互接口也让很多同学遇到了困难，结果就是只有少数同学很顺利地完成了所有任务，一些同学在不理解这些 API 的工作方式的情况下直接拿代码来就用，出现一些问题后就一筹莫展，这是我们不希望看到的一种情况，况且这部分知识与网络原理课程关系不大，日后也基本不会接触。今年我们采用的 `libpcap` 以一个更底层的方式进行收发，绕过了操作系统的 IP 层，这样可以避开 Raw Socket 的一些限制，不过也多了自行维护 ARP 的负担。同时今年新增了硬件路由器实验的组，为了把二者统一，我们设计了 HAL 库，它维护了 ARP 的信息，在 Linux 等平台下用 `libpcap`，在 Xilinx 平台下用 IP 核的寄存器，和 stdio 后端用于在线评测。我们期望通过这些方法减少大家的负担。

Q: 我没有趁手的 Linux 环境，我可以用 WSL 吗

A: 由于 WSL1 没有实现 pcap ，如果使用 Linux 后端，即使 sudo 运行也会报告找不到可以抓包的网口，所以你只能用文件后端进行测试。如果你使用 WSL2，应当可以正常的使用 Linux 后端的所有功能（但不保证没有问题）。

Q: 有时候会出现 `pcap_inject failed with send: Message too long` ，这是什么情况？

A: 这一般是因为传给 `HAL_SendIPPacket` 的长度参数大于网口的 MTU，请检查你传递的参数是否正确。需要注意的是，在一些情况下，在 Linux 后端中， `HAL_ReceiveIPPacket` 有时候会返回一个长度大于 MTU 的包，这是 TSO (TCP Segment Offload) 或者类似的技术导致的（在网卡中若干个 IP 包被合并为一个）。你可以用 `ethtool -K 网口名称 tso off` 来尝试关闭 TSO ，然后在 `ethtool -k 网口名称` 的输出中找到 `tcp-segmentation-offload: on/off` 确认一下是否成功关闭。

Q: RIP 协议用的是组播地址，但组播是用 IGMP 协议进行维护的，这个框架是怎么解决这个问题的？

A: 在 Linux 和 macOS 后端的 `HAL_Init` 函数中，它会向所有网口都发一份 `IGMP Membership Join group 224.0.0.9` 表示本机进入了 RIP 协议的对应组播组之中。为了简化流程，退出时不会发送 Leave Group 的消息，你可以选择手动发送。

Q: 我通过 `veth` 建立了两个 netns 之间的连接，路由器也写好了， RIP 可以通， ICMP 也没问题，但就是 TCP 不工作，抓包也能看到 SYN 但是看不到 SYN+ACK ，这是为啥？

A: 这是因为 Linux 对于网卡有 TX Offload 机制，对于传输层协议的 Checksum 可以交由硬件计算；因此在经过 `veth` 转发时，TCP Checksum 一般是不正确的，这有可能引起一些问题。解决方案和上面类似，用 `ethtool -K veth名称 tx off` 即可，注意 veth 的两侧都要配置。

Q: 这个实验怎么要用到怎么多工具啊？我好像都没学过，这不是为难我吗？

A: 实验所使用的大部分工具相信同学们在若干先前已经修过的课程中已经有所接触，如 Git（软件工程、编译原理）、Make（面向对象程序设计基础）、Python（程序设计训练）、SSH（高性能计算导论）等，只有 Wireshark 和 iproute2 才是完成此次实验需要额外学习的。Wireshark 能帮助同学们完成调试，iproute2 是管理 Linux 操作系统网络的必备工具，我们在下面的附录中提供了一份简短的使用说明。学习并掌握工具的使用方法会更有利于完成实验，这里不做强制要求。另外，物理系和工物系的小学期课程实验物理的大数据方法上课内容囊括了 Git、Make、Python、SSH 和 Linux。

Q: 我听说过转发表这个概念，它和路由表是什么关系？

A: 实际上这两个是不一样的，路由协议操作路由表，转发操作查询转发表，转发表从路由表中来。但软件实现的路由器其实可以不对二者进行区分，所以在文档里统称为路由表。在 router.h 里的 RoutingTableEntry 只有转发需要的内容，但为了支持 RIP 协议，你还需要额外添加一些字段，如 metric 等等。

Q: 树莓派和计算机组成原理用的板子有什么区别？

A: 树莓派就是一个小型的计算机，只不过指令集是 ARM ，其余部分用起来和笔记本电脑没有本质区别；计算机组成原理的板子核心是 FPGA ，你需要编写 Verilog 代码对它进行编程。

Q: 我在树莓派写的可以工作的代码，放到我的 x86 电脑上跑怎么就不工作了呢？或者反过来，我在 x86 电脑上写的可以工作的代码，放到树莓派上怎么就不工作了呢？

A: 一个可能的原因是代码出现了 Undefined Behavior ，编译器在不同架构下编译出不同的代码，导致行为不一致。可以用 UBSan 来发现这种问题。

Q: 我用 ssh 连不上树莓派，有什么办法可以进行诊断吗？

A: 可以拿 HDMI 线把树莓派接到显示器上，然后插上 USB 的键盘和鼠标，登录进去用 `ip` 命令看它的网络情况。网络连接方面，可以用网线连到自己的电脑或者宿舍路由器上，也可以连接到 Wi-Fi 。如果没有显示器，也可以用 USB 转串口，把串口接到树莓派对应的引脚上。

Q: 我在 macOS 上安装了 Wireshark，但是报错找不到 tshark ？

A: tshark 可能被安装到了 /Applications/Wireshark.app/Contents/MacOS/tshark 路径下，如果存在这个文件，把目录放到 PATH 环境变量里就可以了。

Q: 为啥要用树莓派呢，电脑上装一个 Linux 双系统或者开个 Linux 虚拟机不好吗？

A: 树莓派可以提供一个统一的环境，而且对同学的电脑的系统和硬盘空间没有什么要求，而虚拟机和双系统都需要不少的硬盘空间。另外，虚拟机的网络配置比树莓派要更加麻烦，一些同学的电脑也会因为没有开启虚拟化或者 Hyper-V 的原因运行不了 VirtualBox 和 VMWare，三种主流的虚拟机软件都有一些不同，让配置变得很麻烦。同时，树莓派的成熟度和文档都比较好，网上有很多完善的资料，学习起来并不困难，硬件成本也不高。

Q: 我在 WSL 下编译 boilerplate，发现编译不通过，`checksum.cpp` 等几个 cpp 文件都不是合法的 cpp 代码。

A: 这是因为在 Windows 里 git clone 的符号链接在 WSL 内看到的是普通文件，建议在 WSL 中进行 git clone 的操作，这样符号链接才是正确的。

## 附录：`ip` 命令的使用

[附录：`ip` 命令的使用](./HowToUseIp.md)

## 附录：树莓派系统的配置和使用

[附录：树莓派系统的配置和使用](./HowToUseRpi.md)

## 附录：make 命令的使用和 Makefile 的编写

[附录：make 命令的使用和 Makefile 的编写](./HowToUseMake.md)

## 附录：出现问题如何调试

[附录：出现问题如何调试](./HowToDebug.md)

## 名词解释

* apt：debian 发行版的包管理器
* brd：broadcast 的缩写
* cmake：一个编译构建系统，可以生成 make、vs 等可以构建的项目文件
* debian：一个操作系统及自由软件的发行版
* dev：device 的缩写，表示设备
* git：一个版本控制系统
* g++：GCC 的一部分，是一个 C++ 语言的编译器
* HAL：硬件抽象层，表示对一类硬件或者平台进行抽象得到的统一的接口
* iface：interface 的缩写
* interface：Linux 下的一个网口，可以是真实的，也可以是虚拟的
* iproute2: Linux 系统下一个网络管理工具
* journalctl：systemd 的查看服务日志的工具
* lab：实验，可以理解为需要实践的作业。
* link：一条链路，比如一条网线连接两台设备
* linux：由 Linus Torvalds 最初编写并主导开发的操作系统内核
* macOS：苹果公司的操作系统，前身是 Mac OS X
* make：一个编译构建系统
* pcap：1. 是一种格式，存储了网络数据 2. 是一个库/工具，提供了从真实网卡上抓取网络数据包的途径
* pip：python 语言的包管理器
* pyshark：在 Python 中使用 tshark 的一个库
* python3：一个编程语言
* raw socket：Linux 提供的一套接口，可以抓取满足特定条件的数据包
* raspbian：基于 debian 的针对树莓派的发行版
* router：路由器，它主要的工作是在网络层上进行 IP 协议的转发。
* submodule：git 在一个仓库中包括另一个仓库的一种方法
* sudo：以 root 权限运行某个程序
* systemctl：systemd 的一个管理程序，可以控制服务的启动和停止
* tcpdump：一个命令行的抓报工具
* tick：时钟滴答的一下响声
* tshark：Wireshark 的 CLI 版本，可以直接在命令行环境下运行
* ubuntu：基于 debian 的以桌面应用为主的发行版
* windows：微软公司的操作系统
* wireshark：一个用户友好的抓包工具，可以对抓到的数据进行深入的解析
* xilinx：赛灵思公司，计算机组成原理课程使用的 FPGA 来自这个公司

## 项目作者

总设计师： @z4yx

后续维护： @Harry-Chen @jiegec

提交贡献： @Konaoo @nzh63 @linusboyle
