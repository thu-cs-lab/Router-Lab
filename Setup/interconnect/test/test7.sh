#!/bin/sh
# Test 7
# Custom client on PC1 <- server on PC2
# You need to ensure that a tftp server is running
rm -rf ../client/test7
echo "content of test7, custom client" > ../server/test7
cd ../client && ip netns exec PC1 ../../../Homework/tftp/pc1/client get fd00::5:1 test7
cat ../client/test7
