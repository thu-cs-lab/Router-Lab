#!/bin/bash
echo "Raspbian buster is expected with bird installed"
echo "Assume eth1 is the interface to R1"
set -v

ip r add 192.168.5.0/24 via 192.168.1.1 dev eth1