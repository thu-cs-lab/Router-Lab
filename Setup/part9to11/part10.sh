#!/bin/bash
echo "Enable part10"
set -v

ip l set lo up
systemctl restart bird
birdc restart all
birdc enable part10
tcpdump -i eth1 -n -l icmp
