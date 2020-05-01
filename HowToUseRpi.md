## 附录：树莓派系统的配置和使用

网上有很多现成的详细的教程，这里还是简单地描述一遍过程，一些细节如果有不一样的地方，请查阅其他文档或者咨询助教。

首先下载 Raspbian 的镜像文件，推荐从[TUNA 镜像地址](https://mirrors.tuna.tsinghua.edu.cn/raspbian-images/raspbian/images/raspbian-2019-09-30/2019-09-26-raspbian-buster.zip)下载。下载完成后会得到一个 zip 格式的压缩包，解压后得到 img 文件。接着使用镜像烧录工具（如 Balena Etcher）把 img 文件写入到 SD 卡中。这个时候你的电脑上应该有一个名为 boot 的盘符，进入它的根目录，新建一个名为 `ssh` 的空文件，注意不要有后缀，它的功能是让树莓派自动启动 SSH 服务器。给树莓派插入 SD 卡，接通电源，应该可以看到红灯常亮，绿灯闪烁，表示正在读取 SD 卡。

接着，拿一条网线，连接你的电脑（或者路由器）和树莓派的网口，这时候应该可以看到网口下面的状态灯亮起。以电脑为例，请打开网络共享（[macOS 参考 1](https://support.apple.com/zh-cn/guide/mac-help/mchlp1540/mac)，[macOS 参考 2](https://medium.com/@tzhenghao/how-to-ssh-into-your-raspberry-pi-with-a-mac-and-ethernet-cable-636a197d055)，[Linux 参考](https://help.ubuntu.com/community/Internet/ConnectionSharing)，[Windows 参考 1](https://answers.microsoft.com/en-us/windows/forum/windows_10-networking/internet-connection-sharing-in-windows-10/f6dcac4b-5203-4c98-8cf2-dcac86d98fb9)，[Windows 参考 2](https://raspberrypi.stackexchange.com/questions/11684/how-can-i-connect-my-pi-directly-to-my-pc-and-share-the-internet-connection) ），让树莓派可以上网，然后要找到树莓派分配到的 IP 地址，可以用 `arp -a` 命令列出各个网口上通过 ARP 发现过的设备，找到其中的树莓派的 IP 地址。记住它，然后用 SSH 的客户端，如 `ssh pi@$raspi_addr` ，其中 `$raspi_addr` 是树莓派的 IP 地址，如 `ssh pi@192.168.2.5` ，密码是 raspberry ，应该就可以登录进去了：

```bash
$ ssh pi@192.168.2.5
pi@192.168.2.5's password:
Linux raspberrypi 4.19.75-v7+ #1270 SMP Tue Sep 24 18:45:11 BST 2019 armv7l

The programs included with the Debian GNU/Linux system are free software;
the exact distribution terms for each program are described in the
individual files in /usr/share/doc/*/copyright.

Debian GNU/Linux comes with ABSOLUTELY NO WARRANTY, to the extent
permitted by applicable law.
Last login: Thu Sep 26 01:31:29 2019
-bash: warning: setlocale: LC_ALL: cannot change locale (en_US.UTF-8)

SSH is enabled and the default password for the 'pi' user has not been changed.
This is a security risk - please login as the 'pi' user and type 'passwd' to set a new password.
```

可以用 ping 命令来确认树莓派已经连上了网络：

```bash
pi@raspberrypi:~ $ ping www.tsinghua.edu.cn
PING www.d.tsinghua.edu.cn (166.111.4.100) 56(84) bytes of data.
64 bytes from www.tsinghua.edu.cn (166.111.4.100): icmp_seq=1 ttl=57 time=71.7 ms
64 bytes from www.tsinghua.edu.cn (166.111.4.100): icmp_seq=2 ttl=57 time=120 ms
64 bytes from www.tsinghua.edu.cn (166.111.4.100): icmp_seq=3 ttl=57 time=101 ms
64 bytes from www.tsinghua.edu.cn (166.111.4.100): icmp_seq=4 ttl=57 time=148 ms
^C
--- www.d.tsinghua.edu.cn ping statistics ---
4 packets transmitted, 4 received, 0% packet loss, time 7ms
rtt min/avg/max/mdev = 71.744/110.426/148.365/27.896 ms
```

这时候可以克隆下本仓库（如果提示找不到 `git` 可以通过 `sudo apt install git` 解决）：

```bash
pi@raspberrypi:~ $ git clone https://github.com/z4yx/Router-Lab.git
Cloning into 'Router-Lab'...
remote: Enumerating objects: 84, done.
remote: Counting objects: 100% (84/84), done.
remote: Compressing objects: 100% (63/63), done.
remote: Total 803 (delta 41), reused 48 (delta 19), pack-reused 719
Receiving objects: 100% (803/803), 205.12 KiB | 54.00 KiB/s, done.
Resolving deltas: 100% (388/388), done.
```

之后如果仓库有一些改动，你可以用 `git pull` 命令来获取更新。然后按照本文最前面描述的命令把所需的依赖都安装好，可能会弹出一些选项，直接按回车继续即可。进入 `Router-Lab/Homework/checksum` 目录，然后用 `make` 编译以确认环境都没有问题：

```bash
pi@raspberrypi:~ $ cd Router-Lab/Homework/checksum/
pi@raspberrypi:~/Router-Lab/Homework/checksum $ make
g++ --std=c++11 -I ../../HAL/include -DROUTER_BACKEND_STDIO -c checksum.cpp -o checksum.o
g++ --std=c++11 -I ../../HAL/include -DROUTER_BACKEND_STDIO -c main.cpp -o main.o
g++ --std=c++11 -I ../../HAL/include -DROUTER_BACKEND_STDIO -c ../../HAL/src/stdio/router_hal.cpp -o hal.o
g++ checksum.o main.o hal.o -o checksum -lpcap
```

然后你可以尝试本地自测：

```bash
pi@raspberrypi:~/Router-Lab/Homework/checksum $ make grade
python3 grade.py
Removing all output files
Running './checksum < data/checksum_input1.pcap > data/checksum_user1.out'
Running './checksum < data/checksum_input2.pcap > data/checksum_user2.out'
Wrong Answer (showing only first 1 packets):
...
Running './checksum < data/checksum_input4.pcap > data/checksum_user4.out'
Passed: 2/4
```

然后你可以插上 USB 网卡，然后用 `ip` 命令来看它的情况：

```bash
pi@raspberrypi:~ $ ip a
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host
       valid_lft forever preferred_lft forever
2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP group default qlen 1000
    link/ether b8:27:eb:96:83:51 brd ff:ff:ff:ff:ff:ff
    inet 192.168.2.5/24 brd 192.168.2.255 scope global noprefixroute eth0
       valid_lft forever preferred_lft forever
    inet6 fe80::c726:a3ed:9c1a:9203/64 scope link
       valid_lft forever preferred_lft forever
3: wlan0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc pfifo_fast state DOWN group default qlen 1000
    link/ether b8:27:eb:c3:d6:04 brd ff:ff:ff:ff:ff:ff
4: eth1: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc pfifo_fast state DOWN group default qlen 1000
    link/ether 40:3c:fc:01:12:d8 brd ff:ff:ff:ff:ff:ff
```

这里的 `eth1` 就是刚刚插上的 USB 网卡，如果还没插上网线或者网线另一头没接上（注意 `NO-CARRIER`），是没法让它变成 `UP` 的：

```bash
pi@raspberrypi:~ $ sudo ip l set eth1 up
pi@raspberrypi:~ $ ip l
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN mode DEFAULT group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP mode DEFAULT group default qlen 1000
    link/ether b8:27:eb:96:83:51 brd ff:ff:ff:ff:ff:ff
3: wlan0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc pfifo_fast state DOWN mode DORMANT group default qlen 1000
    link/ether b8:27:eb:c3:d6:04 brd ff:ff:ff:ff:ff:ff
4: eth1: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc pfifo_fast state DOWN mode DEFAULT group default qlen 1000
    link/ether 40:3c:fc:01:12:d8 brd ff:ff:ff:ff:ff:ff
```

接上以后，我们可以让它变成 `UP` 状态：

```bash
pi@raspberrypi:~ $ sudo ip l set eth1 up
pi@raspberrypi:~ $ ip l
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN mode DEFAULT group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP mode DEFAULT group default qlen 1000
    link/ether b8:27:eb:96:83:51 brd ff:ff:ff:ff:ff:ff
3: wlan0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc pfifo_fast state DOWN mode DORMANT group default qlen 1000
    link/ether b8:27:eb:c3:d6:04 brd ff:ff:ff:ff:ff:ff
4: eth1: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP mode DEFAULT group default qlen 1000
    link/ether 40:3c:fc:01:12:d8 brd ff:ff:ff:ff:ff:ff
```

接着编译 `Example`，来确认 `HAL` 可以正确收发数据：

```bash
pi@raspberrypi:~ $ cd ~/Router-Lab
pi@raspberrypi:~/Router-Lab $ mkdir build
pi@raspberrypi:~/Router-Lab $ cd build
pi@raspberrypi:~/Router-Lab/build $ cmake .. -DBACKEND=Linux
...
pi@raspberrypi:~/Router-Lab/build $ make
...
pi@raspberrypi:~/Router-Lab/build $ sudo ./Example/capture
HAL_Init: found MAC addr of interface eth1
HAL_Init: pcap capture enabled for eth1
HAL_Init: pcap capture disabled for eth2, either the interface does not exist or permission is denied
HAL_Init: pcap capture disabled for eth3, either the interface does not exist or permission is denied
HAL_Init: pcap capture disabled for eth4, either the interface does not exist or permission is denied
HAL_Init: Joining RIP multicast group 224.0.0.9 for eth1
HAL init: 0
0: 40:3C:FC:01:12:D8
1: 00:00:00:00:00:00
2: 00:00:00:00:00:00
3: 00:00:00:00:00:00
Got IP packet of length 1230 from port 0
Src MAC: 48:BF:6B:ED:1B:F8 Dst MAC: 01:00:5E:00:00:FB
Data: 45 00 04 CE 04 D5 00 00 FF 11 F9 38 C0 A8 17 6D E0 00 00 FB 14 E9 14 E9 04 BA 6B 6B 00 00 84 00 00 00 00 16 00 00 00 00 04 5F 68 61 70 04 5F 74 63 70 05 6C 6F 63 61 6C 00 00 0C 00 01 00 00 70 80 00 21 0F 48
...
```

可以看到，HAL 成功获取了 eth1 的 MAC 地址信息，并且从中抓取到了数据。当你插上更多 USB 网卡的时候，可以获取到 eth2 eth3 eth4 的 MAC 地址信息，也能从它们收发以太网帧。

如果这一步失败了，可能是你的 USB 网卡对应的网口名称并不是 eth1-4 ，这时候你可以编辑 `HAL/src/linux/platform/standard.h`，选择一个无用的网口名称替换掉，然后重新编译。

如果你想基于 `Homework/boilerplate` 来实现最终的路由器，在完成作业题后，到 `Homework/boilerplate` 目录下修改代码、编译并运行即可：

```bash
pi@raspberrypi:~/Router-Lab $ cd Homework/boilerplate/
pi@raspberrypi:~/Router-Lab/Homework/boilerplate $ make
...
pi@raspberrypi:~/Router-Lab/Homework/boilerplate $ sudo ./boilerplate
HAL_Init: found MAC addr of interface eth1
HAL_Init: pcap capture enabled for eth1
HAL_Init: pcap capture disabled for eth2, either the interface does not exist or permission is denied
HAL_Init: pcap capture disabled for eth3, either the interface does not exist or permission is denied
HAL_Init: pcap capture disabled for eth4, either the interface does not exist or permission is denied
HAL_Init: Joining RIP multicast group 224.0.0.9 for eth1
Timer
Timer
Timer
```