CXX ?= g++
LAB_ROOT ?= ../..
CXXFLAGS ?= --std=c++11 -I $(LAB_ROOT)/HAL/include
LDFLAGS ?=

.PHONY: all clean grade
all: lookup

clean:
	rm -f *.o lookup std

grade: lookup
	python3 grade.py

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

common.o: $(LAB_ROOT)/HAL/src/common.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

lookup: lookup.o main.o common.o
	$(CXX) $^ -o $@ $(LDFLAGS) 

std: std.o main.o common.o
	$(CXX) $^ -o $@ $(LDFLAGS) 
