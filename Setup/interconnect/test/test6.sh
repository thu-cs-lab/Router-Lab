#!/bin/sh
# Test 6
# Custom client on PC1 <- Custom server on PC2
# You need to ensure that a custom server is running
rm -rf ../client/test6
echo "content of test6, custom client & custom server" > ../server/test6
cd ../client && ip netns exec PC1 ../../../Homework/tftp/pc1/client get fd00::5:1 test6
cat ../client/test6