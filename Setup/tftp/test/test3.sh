#!/bin/sh
# Test 3
# Custom client -> Standard server
# You need to ensure that a standard server is running
rm -rf ../server/test3
echo "content of test3, custom client & standard server" > ../client/test3
cd ../client && ip netns exec PC1 ../../../Homework/tftp/pc1/client put fd00::3:2 test3
cat ../server/test3