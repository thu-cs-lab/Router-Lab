#!/bin/bash
echo "Raspbian buster is expected with bird installed"
echo "Assume eth1 is the interface to R3"
set -v

# Setup dhcpcd config
dir=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cp $dir/dhcpcd-pc2.conf /etc/dhcpcd.conf
systemctl disable bird
systemctl stop bird
systemctl restart dhcpcd