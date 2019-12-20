#!/bin/bash
set -v

ip netns exec PC2 iperf3 -s -1 >/dev/null &
ip netns exec PC3 iperf3 -s -1 >/dev/null &
sleep 1
ip netns exec PC1 iperf3 -c 192.168.6.3 &
ip netns exec PC4 iperf3 -c 192.168.7.3 &

sleep 11

killall iperf3
