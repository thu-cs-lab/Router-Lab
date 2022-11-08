#!/bin/sh
# Test 5
# Ping fd00::5:1 with hop limit = 3
ip netns exec PC1 ping6 -c 4 -t 3 fd00::5:1