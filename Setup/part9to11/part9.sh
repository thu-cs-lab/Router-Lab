#!/bin/bash
echo "Enable part9"
set -v

ip l set lo up
systemctl restart bird
birdc restart all
birdc enable part9
tcpdump -i eth1 -n -l icmp
