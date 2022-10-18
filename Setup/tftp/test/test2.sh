#!/bin/sh
# Test 2
# Custom client <- Standard server
# You need to ensure that a standard server is running
rm -rf ../client/test2
echo "content of test2, custom client & standard server" > ../server/test2
cd ../client && ip netns exec PC1 ../../../Homework/tftp/pc1/client get fd00::3:2 test2
cat ../client/test2