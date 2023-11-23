#!/bin/sh
# Test 6
# Check route
ip netns exec R1 ip -6 route
ip netns exec R3 ip -6 route