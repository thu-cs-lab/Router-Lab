#!/bin/bash
echo "Enable part9 and disable part{7,8}"
set -v

systemctl restart bird
birdc restart all
birdc enable part9
birdc disable part7
birdc disable part8
tcpdump -i eth1 -n -l icmp