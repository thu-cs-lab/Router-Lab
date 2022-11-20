#!/bin/bash
# Run standard RIPng on r2
# For BIRD v1.6

ip netns exec R2 sysctl net.ipv6.conf.r2r1.disable_ipv6=0
ip netns exec R2 sysctl net.ipv6.conf.r2r1.accept_ra=0
ip netns exec R2 ip a add fd00::3:2/112 dev r2r1
ip netns exec R2 ip r add fd00::1:0/112 via fd00::3:1 dev r2r1
ip netns exec R2 sysctl net.ipv6.conf.r2r3.disable_ipv6=0
ip netns exec R2 ip a add fd00::4:1/112 dev r2r3
# enable IPv6 forwarding
ip netns exec R2 sysctl -w net.ipv6.conf.all.forwarding=1
ip netns exec R2 bird6 -c bird-r2.conf -d -s bird-r2.ctl
