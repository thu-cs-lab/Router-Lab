# router

在这一阶段，你可以复用前面四个作业的代码，实现一个完整的路由器。你需要修改 `main.cpp` ，在所提供的框架上进行完善。

所需要实现的功能请参考代码注释和实验文档。

## Makefile 的使用

当前目录下，有四个目录，分别对应了不同的路由器配置，其中 `r1-r3` 分别对应在 R1 R2 R3 位置的路由器的配置，`custom` 目录对应自定义的配置，用于调试，配置方法见源代码。在对应目录下运行 `make` 就可以得到对应版本的路由器程序。

编译的时候可能会出现如下的 warning：

```
/usr/lib/gcc/aarch64-linux-gnu/6/../../../aarch64-linux-gnu/libpcap.a(nametoaddr.o): In function `pcap_nametoaddrinfo':
(.text+0x94): warning: Using 'getaddrinfo' in statically linked applications requires at runtime the shared libraries from the glibc version used for linking
/usr/lib/gcc/aarch64-linux-gnu/6/../../../aarch64-linux-gnu/libpcap.a(nametoaddr.o): In function `pcap_nametoaddr':
(.text+0x8): warning: Using 'gethostbyname' in statically linked applications requires at runtime the shared libraries from the glibc version used for linking
/usr/lib/gcc/aarch64-linux-gnu/6/../../../aarch64-linux-gnu/libpcap.a(nametoaddr.o): In function `pcap_nametonetaddr':
(.text+0xd8): warning: Using 'getnetbyname' in statically linked applications requires at runtime the shared libraries from the glibc version used for linking
/usr/lib/gcc/aarch64-linux-gnu/6/../../../aarch64-linux-gnu/libpcap.a(nametoaddr.o): In function `pcap_nametoproto':
(.text+0x368): warning: Using 'getprotobyname' in statically linked applications requires at runtime the shared libraries from the glibc version used for linking
/usr/lib/gcc/aarch64-linux-gnu/6/../../../aarch64-linux-gnu/libpcap.a(nametoaddr.o): In function `pcap_nametoport':
(.text+0x124): warning: Using 'getservbyname' in statically linked applications requires at runtime the shared libraries from the glibc version used for linking
```

这是因为，在 TanLabs 评测中，编译机和评测机环境是不一样的，所以采用了静态编译的方式，而静态编译的话，一些功能会不能使用，不过好消息是，这些功能在路由器里不会用到，所以直接忽略这些 warning 即可。
