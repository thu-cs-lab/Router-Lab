#!/bin/bash
set -v

ip netns exec PC2 iperf3 -s &
sleep 1
ip netns exec PC1 iperf3 -c 192.168.6.3 -u -l 16 -t 5 -b 1G

killall iperf3
