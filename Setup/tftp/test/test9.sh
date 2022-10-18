#!/bin/sh
# Test 9
# Custom client <- Custom server
# Benchmark
# You need to ensure that a custom server is running
rm -rf ../client/test9
dd if=/dev/zero of=../server/test9 bs=1024 count=1024
cd ../client && time ip netns exec PC1 ../../../Homework/tftp/pc1/client get fd00::3:2 test9
ls -alh ../client/test9