#!/bin/bash
echo "Raspbian buster is expected with bird installed"
echo "Assume eth1 is the interface to R1"
set -v

# Setup dhcpcd config
dir=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cp $dir/dhcpcd-pc1.conf /etc/dhcpcd.conf
cp $dir/conf-part7.conf /etc/bird/
cp $dir/conf-part8.conf /etc/bird/
cp $dir/conf-part9.conf /etc/bird/
cp $dir/bird1.conf /etc/bird/bird.conf
systemctl disable bird
systemctl stop bird
systemctl restart dhcpcd