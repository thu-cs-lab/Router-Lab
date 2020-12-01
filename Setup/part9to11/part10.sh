#!/bin/bash
echo "Enable part10"
set -v

systemctl restart bird
ip l set lo up
birdc restart all
birdc enable part10
tcpdump -i eth1 -n -l icmp
