#!/bin/sh
# Test 7
# Custom client on PC1 -> server on PC2
# You need to ensure that a tftp server is running
rm -rf ../server/test7
echo "content of test7, custom client" > ../client/test7
cd ../client && ip netns exec PC1 ../../../Homework/tftp/pc1/client put fd00::5:1 test7
cat ../server/test7