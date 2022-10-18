#!/bin/sh
# Test 7
# Custom client <- Custom server
# You need to ensure that a custom server is running
rm -rf ../client/test7
echo "content of test7, custom client & custom server" > ../server/test7
cd ../client && ip netns exec PC1 ../../../Homework/tftp/pc1/client get fd00::3:2 test7
cat ../client/test7