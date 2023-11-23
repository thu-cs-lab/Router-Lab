#!/bin/sh
# Test 9
# Standard client on PC1 <- server on PC2
# Benchmark
# You need to ensure that a tftp server is running
rm -rf ../client/test9
dd if=/dev/zero of=../server/test9 bs=1024 count=1024
cd ../client && time ip netns exec PC1 sh -c "echo \"get test9\" | tftp fd00::5:1"
ls -alh ../client/test9
