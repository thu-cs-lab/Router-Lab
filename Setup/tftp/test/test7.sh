#!/bin/sh
rm -rf ../client/test7
echo "content of test7, custom client & custom server" > ../server/test7
cd ../client && ip netns exec PC1 ../../../Homework/tftp/pc1/client get fd00::3:2 test7
cat ../client/test7