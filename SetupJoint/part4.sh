#!/bin/bash
set -v

ip netns exec PC2 iperf -s &
ip netns exec PC3 iperf -s &
ip netns exec PC1 iperf -c 192.168.6.3 -d -t 10 &
ip netns exec PC4 iperf -c 192.168.7.3 -d -t 10 &

sleep 11

killall iperf3
