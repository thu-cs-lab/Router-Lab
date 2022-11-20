#!/bin/sh
# Start standard tftp server on pc2
# Setup ipv6 address and route
ip netns exec PC2 sysctl net.ipv6.conf.pc2r3.disable_ipv6=0
ip netns exec PC2 ip a add fd00::5:1/112 dev pc2r3
ip netns exec PC2 ip l set pc2r3 up
ip netns exec PC2 ip -6 r add default via fd00::5:2
# Start tftpd-hpa server
cd ../server && ip netns exec PC2 /usr/sbin/in.tftpd --foreground --listen --address :69 --verbose --secure --create --user root .