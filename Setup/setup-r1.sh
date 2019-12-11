#!/bin/bash
echo "Raspbian buster is expected with bird installed"
echo "Assume eth1 is the interface to R2"
dir=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cp $dir/bird.conf /etc/bird/bird.conf
cp $dir/dhcpcd-r1.conf /etc/dhcpcd.conf
systemctl restart bird
systemctl restart dhcpcd
ip netns delete PC1
ip netns add PC1
ip l add veth-r1 type veth peer name veth-pc1
ip l set veth-pc1 netns PC1
ip a add 192.168.1.1/24 dev veth-r1
ip netns exec PC1 ip a add 192.168.1.2/24 dev veth-pc1
ip netns exec PC1 ip l set veth-pc1 up
ip netns exec PC1 ip r add default via 192.168.1.1
ip r del 192.168.1.0/24 dev veth-r1
ip r add 192.168.1.0/24 dev veth-r1
echo 1 > /proc/sys/net/ipv4/conf/all/forwarding