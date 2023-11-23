#!/bin/sh
# Test 8
# Standard client on PC1 -> server on PC2
# You need to ensure that a tftp server is running
rm -rf ../server/test8
echo "content of test8, custom client" > ../client/test8
cd ../client && ip netns exec PC1 sh -c "echo \"put test8\" | tftp fd00::5:1"
cat ../server/test8
