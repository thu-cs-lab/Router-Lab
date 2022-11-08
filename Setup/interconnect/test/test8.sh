#!/bin/sh
# Test 8
# Custom client on PC1 <- server on PC2
# Benchmark
# You need to ensure that a tftp server is running
rm -rf ../client/test8
dd if=/dev/zero of=../server/test8 bs=1024 count=1024
cd ../client && time ip netns exec PC1 ../../../Homework/tftp/pc1/client get fd00::5:1 test8
ls -alh ../client/test8