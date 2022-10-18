#!/bin/sh
# Test 3
# Check dhcpv6 client
ip netns exec PC1 dhcpcd -6 -1 -B -C resolv.conf -d pc1r1
ip netns exec PC1 ip -6 a