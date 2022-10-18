#!/bin/sh
# Test 4
# Check TCP
echo "test" | ip netns exec PC1 nc -6 -l -p 80 -N &
NC_PID=$!
timeout 4s ip netns exec PC2 nc fd00::1:2 80
kill $NC_PID

echo "test" | ip netns exec PC2 nc -6 -l -p 80 -N &
NC_PID=$!
timeout 4s ip netns exec PC1 nc fd00::5:1 80
kill $NC_PID