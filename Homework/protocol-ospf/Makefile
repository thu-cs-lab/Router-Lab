CXX ?= g++
LAB_ROOT ?= ../..
BACKEND ?= STDIO
CXXFLAGS ?= --std=c++11 -I $(LAB_ROOT)/HAL/include -DROUTER_BACKEND_$(BACKEND) -I../lookup
LDFLAGS ?= -lpcap

.PHONY: all clean grade
all: protocol_ospf

clean:
	rm -f *.o protocol_ospf std

grade: protocol_ospf
	python3 grade.py

%.o: %.cpp protocol_ospf.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

hal.o: $(LAB_ROOT)/HAL/src/stdio/router_hal.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

common.o: $(LAB_ROOT)/HAL/src/common.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

lookup.o: ../lookup/lookup.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

protocol_ospf: protocol_ospf.o main.o hal.o lookup.o common.o ospf_lsa_csum.o
	$(CXX) $^ -o $@ $(LDFLAGS) 

std: std.o main.o hal.o lookup.o common.o
	$(CXX) $^ -o $@ $(LDFLAGS) 
