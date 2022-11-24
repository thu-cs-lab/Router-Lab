#!/bin/sh
# Test 6
# Standard client on PC1 <- server on PC2
# You need to ensure that a tftp server is running
rm -rf ../client/test6
echo "content of test6, standard client" > ../server/test6
cd ../client && ip netns exec PC1 sh -c "echo \"get test6\" | tftp fd00::5:1"
cat ../client/test6