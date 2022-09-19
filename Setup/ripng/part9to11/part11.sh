#!/bin/bash
echo "Enable part11"
set -v

ip l set lo up
systemctl restart bird
birdc restart all
birdc enable part11
tcpdump -i eth1 -n -l icmp
