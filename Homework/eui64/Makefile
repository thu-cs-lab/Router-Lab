CXX ?= g++
LAB_ROOT ?= ../..
CXXFLAGS ?= --std=c++11 -I $(LAB_ROOT)/HAL/include
LDFLAGS ?=

.PHONY: all clean grade
all: eui64

clean:
	rm -f *.o eui64 std

grade: eui64
	python3 grade.py

%.o: %.cpp eui64.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

eui64: eui64.o main.o
	$(CXX) $^ -o $@ $(LDFLAGS) 

std: std.o main.o
	$(CXX) $^ -o $@ $(LDFLAGS) 
