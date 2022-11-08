#!/bin/bash
# Run standard RIPng on r3
# For BIRD v1.6

ip netns exec R3 sysctl net.ipv6.conf.r3r2.disable_ipv6=0
ip netns exec R3 ip a add fd00::4:2/112 dev r3r2
ip netns exec R3 sysctl net.ipv6.conf.r3pc2.disable_ipv6=0
ip netns exec R3 ip a add fd00::5:2/112 dev r3pc2

# enable IPv6 forwarding
ip netns exec R3 sysctl -w net.ipv6.conf.all.forwarding=1
ip netns exec R3 bird6 -c bird-r3.conf -d -s bird-r3.ctl

