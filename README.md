# Router-Lab

这里是 2019 年网络原理课程原理课程实验采用的框架。它有以下的设计目标：

1. 降低难度，把与底层打交道的部分抽象成通用的接口，减少学习底层 API 的负担
2. 复用代码，在一个平台上编写的程序可以直接用于其他平台
3. 方便测试，提供文件读写 PCAP 的后端，可以直接用数据进行黑箱测试

请仔细阅读下面的文本，在问问题时，请把以下每个括号中的 **暗号** 按顺序连在一起重复一遍然后再说你的问题。

本文档默认你已经在软件工程、编译原理、程序设计训练等课程中已经学习到了足够的 Git 、Make 、Python3 和 Linux 的使用知识，如果用的是 Windows 系统，你可以在下发的树莓派上进行以下所有相关的操作。

如果你运行的是 Debian 发行版，你可以一把梭地安装需要的依赖：

```shell
sudo apt install git make python3 python3-pip libpcap-dev wireshark iproute2
pip3 install pyshark
```

其他发行版也有类似的包管理器安装方法。

## 如何使用框架

这个框架主要分为两部分，一部分是硬件抽象库，即 HAL （Hardware Abstraction Layer），它提供了数个后端，可以在不修改用户代码的情况下把程序运行在不同的平台上；另一部分是数个小实验，它们对你所需要实现的路由器的几个关键功能进行了针对性的测试，采用文件输入输出的黑盒测试方法，在真机调试之前就可以进行解决很多问题。

（暗号：我）

第一步是克隆本仓库：

```shell
git clone https://github.com/z4yx/Router-Lab.git
cd Router-Lab
git submodule update --init --recursive
```

如果到 GitHub 的网络情况不好，也可以从 `https://git.tsinghua.edu.cn/Router-Lab/Router-Lab.git` 克隆，我们会保证两个地址内容一致。

之后如果这个仓库的代码有什么更新，请运行 `git pull` 进行更新。

### 如何使用 HAL 

在 `HAL` 目录中，是完整的 `HAL` 的源代码。它包括一个头文件 `router_hal.h` 和若干后端的源代码。

如果你有过使用 CMake 的经验，那么建议你采用 CMake 把 HAL 和你的代码链接起来。编译的时候，需要选择 HAL 的后端，可供选择的一共有：

1. Linux: 用于 Linux 系统，基于 libpcap，发行版一般会提供 `libpcap-dev` 或类似名字的包，安装后即可编译。
2. macOS: 用于 macOS 系统，同样基于 libpcap，安装方法类似于 Linux 。
3. stdio: 直接用标准输入输出，也是采用 pcap 格式，按照 VLAN 号来区分不同 interface。
4. Xilinx: 在 Xilinx FPGA 上的一个实现，中间涉及很多与设计相关的代码，并不通用，仅作参考，对于想在 FPGA 上实现路由器的组有一定的参考作用。（暗号：认）

后端的选择方法如下：

```shell
mkdir build
cd build
cmake .. -DBACKEND=Linux
make router_hal
```

其它后端类似设置即可。

如果你不想用 CMake ，你可以直接把 router_hal.h 放到你的 Header Include Path 中，然后把对应后端的文件（如 `HAL/src/linux/router_hal.cpp`）编译并链接进你的程序，同时在编译选项中写 `-DROUTER_BACKEND_LINUX` （即 ROUTER_BACKEND_ 加上后端的大写形式）。可以参考 `Homework/checksum/Makefile` 中相关部分。

在这个时候，你应该可以通过 HAL 的编译。

特别地，由于 Linux/macOS 后端需要配置 interface 的名字，默认情况下采用的是 `eth0-3`（macOS 则是 `en0-3`） 的命名，如果与实际的不符（可以采用 `ifconfig` 或者 `ip a` 命令查看），可以直接修改 `HAL/src/linux/platform/standard.h`（macOS 则是 `HAL/src/macOS/router_hal.cpp`） 或者修改 `HAL/src/linux/platform/testing.h` 并在编译选项中打开 `-DHAL_PLATFORM_TESTING` 进行配置。如果配置不正确，可能会出现一些接口永远收不到，也发不出数据的情况。

### HAL 提供了什么

HAL 即 Hardware Abstraction Layer 硬件抽象层，顾名思义，是隐藏了一些底层细节，简化同学的代码设计。它有以下几点的设计：

1. 所有函数都设计为仅在单线程运行，不支持并行
2. 从 IP 层开始暴露给用户，由框架处理 ARP 和收发以太网帧的具体细节
3. 采用轮询的方式进行 IP 报文的收取
4. 尽量用简单的方法实现，性能不是重点

它提供了以下这些函数：

1. HAL_Init: 使用 HAL 库的第一步，必须调用且仅调用一次，需要提供每个网口上绑定的 IP 地址，第一个参数表示是否打开 HAL 的测试输出，十分建议在调试的时候打开它
2. HAL_GetTicks：获取从启动到当前时刻的毫秒数
3. HAL_ArpGetMacAddress：从 ARP 表中查询 IPv4 地址对应的 MAC 地址，在找不到的时候会发出 ARP 请求
4. HAL_GetInterfaceMacAddress：获取指定网口上绑定的 MAC 地址
5. HAL_ReceiveIPPacket：从指定的若干个网口中读取一个 IPv4 报文，并得到源 MAC 地址和目的 MAC 地址等信息
6. HAL_SendIPPacket：向指定的网口发送一个 IPv4 报文

这些函数的定义和功能都在 `router_hal.h` 详细地解释了，请阅读函数前的文档。为了易于调试，HAL 没有实现 ARP 表的老化，你可以自己在代码中实现，不难。

仅通过这些函数，就可以实现一个软路由。我们在 `Example` 目录下提供了一些例子，它们会告诉你 HAL 库的一些基本使用范式：

1. Shell：提供一个可交互的 shell ，可能需要用 root 权限运行，展示了 HAL 库几个函数的使用方法，可以输出当前的时间，查询 ARP 表，查询端口的 MAC 地址，进行一次抓包并输出它的内容，向网口写随机数据等等；它需要 `libncurses-dev` 和 `libreadline-dev` 两个额外的包来编译
2. Broadcaster：一个粗糙的“路由器”，把在每个网口上收到的 IP 包又转发到所有网口上（暗号：真）
3. Capture：仅把抓到的 IP 包原样输出

如果你使用 CMake，可以用类似上面编译 HAL 库的方法编译这三个例子。如果不想使用 CMake，可以基于 `Homework/checksum/Makefile` 修改出适合例子的 Makefile 。它们可能都需要 root 权限运行，并在运行的适合你可以打开 Wireshark 等抓包工具研究它的具体行为。

这些例子可以用于检验环境配置是否正确，如 Linux 下网卡名字的配置、是否编译成功等等。比如在上面的 Shell 程序中输入 `mac 0` `mac 1` `mac 2` 和 `mac 3`，它会输出对应网口的 MAC 地址，如果输出的数据和你用 `ip l`（macOS 可以用 `ifconfig`） 看到的内容一致，那基本说明你配置没有问题了。

你也可以利用 HAL 本身的调试输出，只需要在运行 `HAL_Init` 的时候设置 `debug` 标志 ，你就可以在 stderr 上看到一些有用的输出。

#### 各后端的自定义配置

各后端有一个公共的设置  `N_IFACE_ON_BOARD` ，它表示 HAL 需要支持的最大的接口数，一般取 4 就是足够了。

在 Linux 后端中，一个很重要的是 `interfaces` 数组，它记录了 HAL 内接口下标与 Linux 系统中的网口的对应关系，你可以用 `ip a` 来列出系统中存在的所有的网口。为了方便开发，我们提供了 `HAL/src/linux/platform/{standard,testing}.h` 两个文件（形如 a{b,c}d 的语法代表的是 abd 或者 acd），你可以通过 HAL_PLATFORM_TESTING 选项来控制选择哪一个。

在 macOS 后端中，类似地你也需要修改 `HAL/src/macOS/router_hal.cpp` 中的 `interfaces` 数组，不过实际上 `macOS` 的网口命名方式比较简单，所以一般不用改也可以碰上对的。

## 如何进行本地自测

在 `Homework` 目录下提供了若干个题目，通过数据测试你的路由器中核心功能的实现。你需要在被标记 TODO 的函数中补全它的功能，通过测试后，就可以更容易地完成后续的实践。

有这些目录：

```
checksum： 计算校验和
forwarding： 转发逻辑
lookup： 路由表查询和更新
protocol： RIP 协议解析和封装
boilerplate： 用以上代码实现一个路由器
```

每个题目都有类似的结构（以 `checksum` 为例）：

```
data： 数据所在的目录
checksum.cpp：你需要修改的地方
grade.py：一个简单的评分脚本，它会编译并运行你的代码，进行评测
main.cpp：用于评测的交互库，你不需要修改它
Makefile：用于编译并链接 HAL 、交互库和你实现的代码
```

使用方法：

```shell
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
    // 初始化 HAL，打开调试信息
    HAL_Init(1, addrs);

    uint64_t last_time = 0;
    while (1) {
      // 获取当前时间，处理定时任务
        uint64_t time = HAL_GetTicks();
        if (time > last_time + 30 * 10000) {
            // 每 30s 做什么
            // 例如：超时？发 RIP Request？
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
            // 3b.1 此时目的 IP 地址不是路由器本身，则调用你编写的 query 函数查询，如果查到目的地址，
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

你可以直接基于 `Homework/boilerplate` 下的代码，把上面的代码实现完全。

### 如何启动并配置一个比较标准的 RIP 实现

你可以用一台 Linux 机器，连接到你的路由器的一个网口上，一边抓包一边运行一个 RIP 的实现。我们提供一个 BIRD（BIRD Internet Routing Daemon，安装方法 `apt install bird`）v2.0 的参考配置，以 Debian 为例，修改文件 `/etc/bird.conf`：

```
# log "bird.log" all; # 可以将 log 输出到文件中
# debug protocols all; # 如果要更详细的信息，可以打开这个

router id 网口IP地址;

protocol device {
}

protocol kernel {
    # 表示 BIRD 会把系统的路由表通过 RIP 发出去，也会把收到的 RIP 信息写入系统路由表
    # 你可以用 `ip route` 命令查看系统的路由表
    persist no;
    learn;
    ipv4 {
        export all;
    };
}

protocol static {
    ipv4 { };
    route 1.2.3.4/32 via "网口名称"; # 添加一个静态路由让路由表非空
}

protocol rip {
    ipv4 {
        import all;
        export all;
    };
    debug all;
    interface "网口名称" {
        version 2;
        update time 5; # 5秒一次更新，方便调试
    };
}
```

如果你用的是 v1.6 版本，上面有一些字段需要修改。

这里的网口名字对应你连接到路由器的网口，也要配置一个固定的 IP 地址，需要和路由器对应网口的 IP 在同一个网段内。配置固定 IP 地址的命令格式为 `ip a add IP地址/前缀长度 dev 网口名称`，你可以用 `ip a` 命令看到所有网口的信息。

启动服务（如 `systemctl start bird`）后，你就可以开始抓包，同时查看 bird 打出的信息（`journalctl -f -u bird`），这对调试你的路由器实现很有帮助。

或者直接运行 BIRD（`bird -c /etc/bird.conf`），可在命令选项中加上 `-d` 方便直接退出进程。若想同时开多个 BIRD 则需要给每个进程指定单独的 socket。

### 如何在一台计算机上进行真实测试

如果你使用软件实现了路由器，你可以用以下的方法只使用一台计算机进行测试：

1. 使用虚拟机安装多个不同的操作系统，并将它们的网络按照需要的拓扑连接，在每一台虚拟机上运行一份你的程序。这一方法思路简单，并且可以做到与真实多机环境完全相同，但可能消耗较多的资源。
2. 使用 Linux 提供的 network namespace 功能，在同一个系统上创建多个相互隔离的网络环境，并使用 veth 将它们恰当地连接起来，在其中运行多份你的程序。这一方法资源占用少，但是对 Linux 使用经验和网络配置知识有较高的需求，建议看下面的 FAQ 避开一些常见问题。

和 network namespace 相关的操作的子命令是 `ip netns`。例如我们想要创建两个 namespace 并让其能够互相通信：

```
ip netns add net0 # 创建名为 "net0" 的 namespace
ip netns add net1
ip link add veth0 type veth peer name veth1 # 创建 veth pair 作为 namespace 之间的通信渠道
ip link set veth0 netns net0 # 将 veth 加入到 namespace 中
ip link set veth1 netns net1
ip netns exec net0 ip link set veth0 up
ip netns exec net0 ip addr add 10.1.1.1/24 dev veth0 # 给 veth 配上 ip 地址
ip netns exec net1 ip link set veth1 up
ip netns exec net1 ip addr add 10.1.1.2/24 dev veth1
```

配置完成后你可以运行 `ip netns exec net0 ping 10.1.1.2` 来测试在 net0 上是否能够 ping 到 net1。

你还可以运行 `ip netns exec net0 [command]` 来执行任何你想在 namespace 下执行的命令，甚至可以运行 `ip netns exec net0 bash` 打开一个网络环境为 net0 的 bash。

## FAQ（暗号：档）

Q：暗号是干嘛的，为啥要搞这一出？

A：总是有同学不认真阅读文档，所以，如果你阅读到了这里，请心里默念暗号：_______

Q：我用的是纯命令行环境，没有 Wireshark 图形界面可以用，咋办？

A：你可以用 tcpdump 代替 Wireshark，它的特点是一次性输出所有内容；或者用 tshark，是 Wireshark 官方的 CLI 版本；也可以用 termshark ，它是 Wireshark 的 TUI 版，操作方式和 Wireshark 是一致的

Q: 运行 grade.py 的时候，提示找不到 tshark ，怎么办？

A: 用你的包管理器安装 wireshark 或者 tshark 都行。如果你在使用 Windows，需要注意 Windows 版的 Wireshark 和 WSL 内部的 Wireshark 是需要分别安装的。

Q：为啥要搞 HAL 啊，去年让大家用 Linux 的 Raw Socket ，不也有人搞出来了吗？

A：我们认为去年的 Linux 的 Raw Socket 是比较古老而且需要同学编写很多人冗余代码的一套 API，另外比较复杂的 Quagga 的交互接口也让很多同学遇到了困难，结果就是只有少数同学很顺利地完成了所有任务，一些同学在不理解这些 API 的工作方式的情况下直接拿代码来就用，出现一些问题后就一筹莫展，这是我们不希望看到的一种情况，况且这部分知识与网络原理课程关系不大，日后也基本不会接触。今年我们采用的 libpcap 以一个更底层的方式进行收发，绕过了操作系统的 IP 层，这样可以避开 Raw Socket 的一些限制，不过也多了自行维护 ARP 的负担。同时今年新增了硬件路由器实验的组，为了把二者统一，我们设计了 HAL 库，它维护了 ARP 的信息，在 Linux 等平台下用 libpcap，在 Xilinx 平台下用 IP 核的寄存器，和 stdio 后端用于在线评测。我们期望通过这些方法减少大家的负担。

Q: 我没有趁手的 Linux 环境，我可以用 WSL 吗

A: 由于 WSL1 没有实现 pcap ，如果使用 Linux 后端，即使 sudo 运行也会报告找不到可以抓包的网口，所以你只能用文件后端进行测试。如果你使用 WSL2，应当可以正常的使用 Linux 后端的所有功能（但不保证没有问题）。

Q: 有时候会出现 `pcap_inject failed with send: Message too long` ，这是什么情况？

A: 这一般是因为传给 `HAL_SendIPPacket` 的长度参数大于网口的 MTU，请检查你传递的参数是否正确。需要注意的是，在一些情况下，在 Linux 后端中， `HAL_ReceiveIPPacket` 有时候会返回一个长度大于 MTU 的包，这是因为 TSO(TCP Segment Offload) 或者类似的技术，在网卡中若干个 IP 包被合并为一个。你可以用 `ethtool -K 网口名称 tso off` 来尝试关闭 TSO ，然后在 `ethtool -k 网口名称` 的输出中找到 `tcp-segmentation-offload: on/off` 确认一下是否成功关闭。

Q: RIP 协议用的是组播地址，但组播是用 IGMP 协议进行维护的，这个框架是怎么解决这个问题的？

A: 在 Linux 和 macOS 后端的 `HAL_Init` 函数中，它会向所有网口都发一份 `IGMP Membership Join group 224.0.0.9` 表示本机进入了 RIP 协议的组之中。为了简化，并没有发 Leave Group 的消息。

Q: 我通过 `veth` 建立了两个 netns 之间的连接，路由器也写好了， RIP 可以通， ICMP 也没问题，但就是 TCP 不工作，抓包也能看到 SYN 但是看不到 SYN+ACK ，这是为啥？

A: 这是因为 `veth` 会跳过 TX 的 Checksum ，然后在转发的时候，TCP Checksum 就不正确，对方虽然收到了，但是并不会回应。解决方案和上面类似，用 `ethtool -K veth名称 tx off` 即可，注意两个网口都要配置。

## 项目作者

总设计师： @z4yx

后续维护： @Harry-Chen @jiegec

提过建议： @Konaoo
