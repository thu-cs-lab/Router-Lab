#!/bin/sh
# Test 8
# Custom client -> Custom server
# You need to ensure that a custom server is running
rm -rf ../server/test8
echo "content of test8, custom client & custom server" > ../client/test8
cd ../client && ip netns exec PC1 ../../../Homework/tftp/pc1/client put fd00::3:2 test8
cat ../server/test8