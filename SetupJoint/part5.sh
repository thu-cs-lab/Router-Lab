#!/bin/bash
set -v

ip netns exec PC2 iperf -s &
ip netns exec PC1 iperf -c 192.168.6.3 -u -l 16 -t 5 -b 1G &

sleep 6

killall iperf3
