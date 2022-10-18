#!/bin/sh
# Test 8
# Benchmark using iperf3
ip netns exec PC2 iperf3 -s &
IPERF_PID=$!
ip netns exec PC1 iperf3 -c fd00::5:1 -O 5 -P 10
kill $IPERF_PID