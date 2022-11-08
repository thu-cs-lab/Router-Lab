#!/bin/sh
# Test 4
# You need to ensure that PC1 receives dynamic IPv6 via dhcpcd
# Ping fd00::3:2
ip netns exec PC1 ping6 -c4 fd00::3:2