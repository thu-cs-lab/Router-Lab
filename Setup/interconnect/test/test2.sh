#!/bin/sh
# Test 2
# Check dhcpv6 client
ip netns exec PC1 sysctl net.ipv6.conf.pc1r1.disable_ipv6=0
ip netns exec PC1 dhcpcd -6 -1 -B -C resolv.conf -d pc1r1
ip netns exec PC1 ip -6 a