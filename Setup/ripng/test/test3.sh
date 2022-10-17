#!/bin/sh
# Test 3
ip netns exec PC1 ping -c 4 fd00::5:1
ip netns exec PC2 ping -c 4 fd00::1:2