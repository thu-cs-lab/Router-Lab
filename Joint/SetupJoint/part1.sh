#!/bin/bash
set -v
ip netns exec PC1 ping -c2 192.168.6.3
ip netns exec PC1 ping -c2 192.168.7.3
ip netns exec PC1 ping -c2 192.168.6.3

ip netns exec PC2 ping -c2 192.168.5.3
ip netns exec PC2 ping -c2 192.168.7.3
ip netns exec PC2 ping -c2 192.168.8.3

ip netns exec PC3 ping -c2 192.168.5.3
ip netns exec PC3 ping -c2 192.168.6.3
ip netns exec PC3 ping -c2 192.168.8.3

ip netns exec PC4 ping -c2 192.168.5.3
ip netns exec PC4 ping -c2 192.168.6.3
ip netns exec PC4 ping -c2 192.168.7.3
