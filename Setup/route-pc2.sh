#!/bin/bash
echo "Raspbian buster is expected with bird installed"
echo "Assume eth1 is the interface to R3"
set -v

ip r add 192.168.1.0/24 via 192.168.5.2 dev eth1
ip r add 192.168.10.1/32 via 192.168.5.2 dev eth1
ip r add 192.168.255.1/32 via 192.168.5.2 dev eth1
ip r add 10.1.2.3/32 via 192.168.5.2 dev eth1
ip r add 10.8.7.6/32 via 192.168.5.2 dev eth1
ip r add 166.111.4.100/32 via 192.168.5.2 dev eth1
ip r add 101.6.4.100/32 via 192.168.5.2 dev eth1
ip r add 59.66.134.1/32 via 192.168.5.2 dev eth1