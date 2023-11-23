#!/bin/sh
# Test 8
# Custom client on PC1 -> server on PC2
# You need to ensure that a tftp server is running
rm -rf ../server/test8
echo "content of test8, custom client" > ../client/test8
cd ../client && ip netns exec PC1 ../../../Homework/tftp/pc1/client put fd00::5:1 test8
cat ../server/test8
