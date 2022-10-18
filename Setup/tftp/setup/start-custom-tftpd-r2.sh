#!/bin/sh
# Start custom tftpd on r2
ip netns exec R2 sysctl net.ipv6.conf.r2r1.disable_ipv6=1
cd ../server && ip netns exec R2 ../../../Homework/tftp/r2/server