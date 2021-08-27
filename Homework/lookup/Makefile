CXX ?= g++
LAB_ROOT ?= ../..
CXXFLAGS ?= --std=c++11
LDFLAGS ?=

.PHONY: all clean grade
all: lookup

clean:
	rm -f *.o lookup std

grade: lookup
	python3 grade.py

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

lookup: lookup.o main.o
	$(CXX) $^ -o $@ $(LDFLAGS) 

std: std.o main.o
	$(CXX) $^ -o $@ $(LDFLAGS) 
