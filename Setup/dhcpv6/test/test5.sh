#!/bin/sh
# Test 5
# Benchmark using iperf3
ip netns exec R2 iperf3 -s &
IPERF_PID=$!
ip netns exec PC1 iperf3 -c fd00::3:2 -O 5 -P 10
kill $IPERF_PID