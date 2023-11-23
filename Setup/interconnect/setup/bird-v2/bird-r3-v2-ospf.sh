#!/bin/bash
# Run standard OSPF on r3
# For BIRD v2

ip netns exec R3 sysctl net.ipv6.conf.r3r2.disable_ipv6=0
ip netns exec R3 ip a add fd00::4:2/112 dev r3r2
ip netns exec R3 sysctl net.ipv6.conf.r3pc2.disable_ipv6=0
ip netns exec R3 ip a add fd00::5:2/112 dev r3pc2

# enable IPv6 forwarding
ip netns exec R3 sysctl -w net.ipv6.conf.all.forwarding=1
ip netns exec R3 bird -c bird-r3-v2-ospf.conf -d -s bird-r3-ospf.ctl

