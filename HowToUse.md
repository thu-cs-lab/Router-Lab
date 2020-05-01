## 如何使用框架

这个框架主要分为两部分，一部分是硬件抽象层，即 HAL （Hardware Abstraction Layer），它提供了数个后端，可以在不修改用户代码的情况下把程序运行在不同的平台上；另一部分是数个小实验，它们对你所需要实现的路由器的几个关键功能进行了针对性的测试，采用文件输入输出的黑盒测试方法，在真机调试之前就可以进行解决很多问题。

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

在 `HAL` 目录中，是完整的 HAL 的源代码。它包括一个头文件 `router_hal.h` 和若干后端的源代码。

如果你有过使用 CMake 的经验，那么建议你采用 CMake 把 HAL 和你的代码链接起来。编译的时候，需要选择 HAL 的后端，可供选择的一共有：

1. Linux: 用于 Linux 系统，基于 libpcap，发行版一般会提供 `libpcap-dev` 或类似名字的包，安装后即可编译。
2. macOS: 用于 macOS 系统，同样基于 libpcap，安装方法类似于 Linux 。
3. stdio: 直接用标准输入输出，也是采用 pcap 格式，按照 VLAN 号来区分不同 interface。
4. Xilinx: 在 Xilinx FPGA 上的一个实现，中间涉及很多与设计相关的代码，并不通用，仅作参考，对于想在 FPGA 上实现路由器的组有一定的参考作用。（暗号：认）

后端的选择方法如下（在 Router-Lab 目录下执行）：

```bash
mkdir build
cd build
cmake .. -DBACKEND=Linux
make router_hal # 编译 HAL 库，生成 ./HAL/librouter_hal.a
make capture # 编译 Example 中的 capture，生成 ./Example/capture
make # 编译 HAL 库和所有 Example
```

其它后端类似设置即可。

如果你不想用 CMake ，你可以直接把 `router_hal.h` 放到你的 Header Include Path 中，然后把对应后端的文件（如 `HAL/src/linux/router_hal.cpp`）编译并链接进你的程序，同时在编译选项中写 `-DROUTER_BACKEND_LINUX` （即 ROUTER_BACKEND_ 加上后端的大写形式）。可以参考 `Homework/checksum/Makefile` 中相关部分。

在这个时候，你应该可以通过 HAL 的编译。

特别地，由于 Linux/macOS 后端需要配置 interface 的名字，默认情况下采用的是 `eth1-4`（macOS 则是 `en0-3`） 的命名，如果与实际的不符（可以采用 `ifconfig` 或者 `ip a` 命令查看），可以直接修改 `HAL/src/linux/platform/standard.h`（macOS 则是 `HAL/src/macOS/router_hal.cpp`） 或者修改 `HAL/src/linux/platform/testing.h` 并在编译选项中打开 `-DHAL_PLATFORM_TESTING` 进行配置。如果配置不正确，可能会出现一些接口永远收不到，也发不出数据的情况。

### HAL 提供了什么

HAL 即 Hardware Abstraction Layer 硬件抽象层，顾名思义，是隐藏了一些底层细节，简化同学的代码设计。它有以下几点的设计：

1. 所有函数都设计为仅在单线程运行，不支持并行
2. 从 IP 层开始暴露给用户，由框架处理 ARP 和收发以太网帧的具体细节
3. 采用轮询的方式进行 IP 报文的收取
4. 尽量用简单的方法实现，而非追求极致性能

它提供了以下这些函数：

1. `HAL_Init`: 使用 HAL 库的第一步，**必须调用且仅调用一次**，需要提供每个网口上绑定的 IP 地址，第一个参数表示是否打开 HAL 的测试输出，十分建议在调试的时候打开它
2. `HAL_GetTicks`：获取从启动到当前时刻的毫秒数
3. `HAL_ArpGetMacAddress`：从 ARP 表中查询 IPv4 地址对应的 MAC 地址，在找不到的时候会发出 ARP 请求
4. `HAL_GetInterfaceMacAddress`：获取指定网口上绑定的 MAC 地址
5. `HAL_ReceiveIPPacket`：从指定的若干个网口中读取一个 IPv4 报文，并得到源 MAC 地址和目的 MAC 地址等信息；它还会在内部处理 ARP 表的更新和响应，需要定期调用
6. `HAL_SendIPPacket`：向指定的网口发送一个 IPv4 报文

这些函数的定义和功能都在 `router_hal.h` 详细地解释了，请阅读函数前的文档。为了易于调试，HAL 没有实现 ARP 表的老化，你可以自己在代码中实现，并不困难。

你可以利用 HAL 本身的调试输出，只需要在运行 `HAL_Init` 的时候设置 `debug` 标志 ，你就可以在 stderr 上看到一些有用的输出。

<details>
    <summary>用 HAL 库编写的例子</summary>

仅通过这些函数，就可以实现一个软路由。我们在 `Example` 目录下提供了一些例子，它们会告诉你 HAL 库的一些基本使用范式：

1. Shell：提供一个可交互的 shell ，可能需要用 root 权限运行，展示了 HAL 库几个函数的使用方法，可以输出当前的时间，查询 ARP 表，查询端口的 MAC 地址，进行一次抓包并输出它的内容，向网口写随机数据等等；它需要 `libncurses-dev` 和 `libreadline-dev` 两个额外的包来编译
2. Broadcaster：一个粗糙的“路由器”，把在每个网口上收到的 IP 包又转发到所有网口上（暗号：真）
3. Capture：仅把抓到的 IP 包原样输出

如果你使用 CMake，可以从上面编译 HAL 库的部分找到编译这三个例子的方法。如果不想使用 CMake，可以基于 `Homework/checksum/Makefile` 修改出适合例子的 Makefile 。它们可能都需要 root 权限运行，并在运行的时候你可以打开 Wireshark 等抓包工具研究它的具体行为。

这些例子可以用于检验环境配置是否正确，如 Linux 下网卡名字的配置、是否编译成功等等。比如在上面的 Shell 程序中输入 `mac 0` `mac 1` `mac 2` 和 `mac 3`，它会输出对应网口的 MAC 地址，如果输出的数据和你用 `ip l`（macOS 可以用 `ifconfig`） 看到的内容一致，那基本说明你配置没有问题了。

</details>

#### 各后端的自定义配置

各后端有一个公共的设置  `N_IFACE_ON_BOARD` ，它表示 HAL 需要支持的最大的接口数，一般取 4 就足够了。

在 Linux 后端中，一个很重要的是 `interfaces` 数组，它记录了 HAL 内接口下标与 Linux 系统中的网口的对应关系，你可以用 `ip l` 来列出系统中存在的所有的网口。为了方便开发，我们提供了 `HAL/src/linux/platform/{standard,testing}.h` 两个文件（形如 a{b,c}d 的语法代表的是 abd 或者 acd），你可以通过 HAL_PLATFORM_TESTING 选项来控制选择哪一个，或者修改/新增文件以适应你的需要。

在 macOS 后端中，类似地你也需要修改 `HAL/src/macOS/router_hal.cpp` 中的 `interfaces` 数组，不过实际上 `macOS` 的网口命名方式比较简单，所以一般不用改也可以碰上对的。