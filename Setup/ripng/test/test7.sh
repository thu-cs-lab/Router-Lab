#!/bin/sh
# Test 7
# Check icmp echo reply
ip netns exec PC1 ping -c 4 fd00::3:2
ip netns exec PC2 ping -c 4 fd00::4:1