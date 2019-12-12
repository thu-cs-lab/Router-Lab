#!/bin/bash
echo "Stop bird and restore dhcpcd config"
set -v

systemctl stop bird
systemctl disable bird
dir=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cp $dir/dhcpcd.conf /etc/dhcpcd.conf
systemctl restart dhcpcd