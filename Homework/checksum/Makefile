CXX ?= g++
LAB_ROOT ?= ../..
CXXFLAGS ?= --std=c++11 -I $(LAB_ROOT)/HAL/include -DROUTER_BACKEND_STDIO
LDFLAGS ?= -lpcap

.PHONY: all
all: checksum

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

hal.o: $(LAB_ROOT)/HAL/src/stdio/router_hal.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

checksum: checksum.o main.o hal.o
	$(CXX) $(LDFLAGS) $^ -o $@