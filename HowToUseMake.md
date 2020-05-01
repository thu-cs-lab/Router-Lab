## 附录：make 命令的使用和 Makefile 的编写

`make` 命令的功能就是按照 Makefile 中编写的规则来生成一些文件，这些文件之间会有依赖的关系，`make` 会安装依赖关系增量地进行生成，达到编译一个完整的程序的目的。下面以 `Homework/boilerplate/Makefile` 举例说明：

```makefile
CXX ?= g++
LAB_ROOT ?= ../..
BACKEND ?= LINUX
CXXFLAGS ?= --std=c++11 -I $(LAB_ROOT)/HAL/include -DROUTER_BACKEND_$(BACKEND)
LDFLAGS ?= -lpcap
```

这一部分定义了若干个变量，左边是 key ，右边是 value ，`$(LAB_ROOT)` 表示变量 `LAB_ROOT` 的内容。条件赋值 `?=` 表示的是只有在第一次赋值时才生效，可以通过 `make CXX=clang++` 来让 CXX 变量的内容变成 clang++。

```makefile
.PHONY: all clean
all: boilerplate

clean:
	rm -f *.o boilerplate std

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

hal.o: $(LAB_ROOT)/HAL/src/linux/router_hal.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

boilerplate: main.o hal.o protocol.o checksum.o lookup.o forwarding.o
	$(CXX) $^ -o $@ $(LDFLAGS)
```

这里出现了一个格式： `xxx: aaa bbb ccc` ，它代表如果要生成 `xxx` ，首先要生成 `aaa` `bbb` 和 `ccc` ，代表了依赖关系，只有 `aaa` `bbb` `ccc` 都生成了，才会生成 `xxx`，另一方面，如果 `xxx` 的更新时间比 `aaa` `bbb` `ccc` 都新，那么就不会重新生成 `xxx` 。紧接着这一行的就是生成的具体过程。

在这里， `.PHONY: all clean` 是特殊的，代表右侧的目标并不是真正的文件，如果没有这一行，并且当前目录有一个名为 `all` 的文件，那么它就不会执行 `all` 内部的命令；加上这一行以后，即使有名为 `all` 的文件，也会尝试去构建。

Make 通过 `%.o` 的格式来支持 wildcard，如 `%.o: %.cpp` 就可以针对所有的 .cpp 文件构建对应的 .o 文件。在命令中，用 `$^` 代表所有依赖， `$@` 代表目标文件， `$<` 代表第一个依赖，如在 `xxx: aaa bbb ccc` 中，`$^` 代表 `aaa bbb ccc` ，`$@` 代表 `xxx` ，`$<` 代表 `aaa`。

以上就涵盖了本实验中 Makefile 用到的所有语法。使用的时候只需要 `make` 就可以了。
