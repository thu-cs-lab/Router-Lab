#!/bin/bash
set -v

ip netns exec PC1 iperf3 -s -1 > /dev/null &
sleep 1
ip netns exec PC2 iperf3 -c 192.168.5.3

killall iperf3
