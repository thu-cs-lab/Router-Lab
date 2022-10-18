#!/bin/sh
# Start standard tftpd on r2
# Setup ipv6 address and route
ip netns exec R2 sysctl net.ipv6.conf.r2r1.disable_ipv6=0
ip netns exec R2 ip a add fd00::3:2/112 dev r2r1
ip netns exec R2 ip l set r2r1 up
ip netns exec R2 ip -6 r add default via fd00::3:1
# Start tftpd-hpa server
cd ../server && ip netns exec R2 /usr/sbin/in.tftpd --foreground --listen --address :69 --verbose --secure --create --user root .