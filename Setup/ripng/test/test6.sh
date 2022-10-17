#!/bin/sh
# Test 6
ip netns exec PC1 ping -c 4 -t 2 fd00::5:1
ip netns exec PC2 ping -c 4 -t 2 fd00::1:2