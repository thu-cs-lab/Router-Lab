CXX ?= g++
LAB_ROOT ?= ../..
BACKEND ?= STDIO
CXXFLAGS ?= --std=c++11 -I $(LAB_ROOT)/HAL/include -DROUTER_BACKEND_$(BACKEND)
LDFLAGS ?= -lpcap

.PHONY: all clean grade
all: checksum

clean:
	rm -f *.o checksum std

grade: checksum
	python3 grade.py

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

hal.o: $(LAB_ROOT)/HAL/src/stdio/router_hal.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

checksum: checksum.o main.o hal.o
	$(CXX) $^ -o $@ $(LDFLAGS) 

std: std.o main.o hal.o
	$(CXX) $^ -o $@ $(LDFLAGS) 
