#!/bin/bash
echo "Enable part9"
set -v

systemctl restart bird
ip l set lo up
birdc restart all
birdc enable part9
tcpdump -i eth1 -n -l icmp
