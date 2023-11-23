#!/bin/sh
# Test 7
# Standard client on PC1 <- server on PC2
# You need to ensure that a tftp server is running
rm -rf ../client/test7
echo "content of test7, standard client" > ../server/test7
cd ../client && ip netns exec PC1 sh -c "echo \"get test7\" | tftp fd00::5:1"
cat ../client/test7
