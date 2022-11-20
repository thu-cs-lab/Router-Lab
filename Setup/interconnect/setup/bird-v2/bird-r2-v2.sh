#!/bin/bash
# Run standard RIPng on r2
# For BIRD v2.x

ip netns exec R2 sysctl net.ipv6.conf.r2r1.disable_ipv6=0
ip netns exec R2 ip a add fd00::3:2/112 dev r2r1
ip netns exec R2 sysctl net.ipv6.conf.r2r3.disable_ipv6=0
ip netns exec R2 ip a add fd00::4:1/112 dev r2r3
ip netns exec R2 bird -c bird-r2-v2.conf -d -s bird-r2.ctl
