#!/bin/sh
# Start standard IPv6 RA server on R1
ip netns exec R1 sysctl net.ipv6.conf.r1pc1.disable_ipv6=0
ip netns exec R1 ip a add fd00::1:1/112 dev r1pc1
ip netns exec R1 ip l set r1pc1 up
ip netns exec R1 sysctl net.ipv6.conf.r1r2.disable_ipv6=0
ip netns exec R1 ip a add fd00::3:1/112 dev r1r2
ip netns exec R1 ip l set r1r2 up
ip netns exec R1 ip -6 r add default via fd00::3:2
# enable IPv6 forwarding
ip netns exec R1 sysctl -w net.ipv6.conf.all.forwarding=1
ip netns exec R1 /usr/sbin/radvd -n -C radvd.conf -p radvd.pid
