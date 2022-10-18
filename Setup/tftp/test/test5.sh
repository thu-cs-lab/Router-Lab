#!/bin/sh
# Test 5
# Standard client <- Custom server
# You need to ensure that a custom server is running
# Setup ipv6 address and route
ip netns exec PC1 sysctl net.ipv6.conf.pc1r1.disable_ipv6=0
ip netns exec PC1 ip a add fd00::1:2/112 dev pc1r1
ip netns exec PC1 ip l set pc1r1 up
ip netns exec PC1 ip -6 r add default via fd00::1:1

rm -rf ../client/test5
echo "content of test5, standard client & custom server" > ../server/test5
cd ../client && ip netns exec PC1 sh -c "echo \"get test5\" | tftp fd00::3:2"
cat ../client/test5