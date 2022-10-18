#!/bin/bash
# Setup netns for individual judge
# IPv6 forwarding is enabled for R1

# netns
for ns in PC1 R1 R2
do
    ip netns delete $ns 2>/dev/null
    sleep 1
    ip netns add $ns
done
echo 'Netns created'

# PC1 <-> R1
ip l del pc1r1 2>/dev/null
ip l add pc1r1 type veth peer name r1pc1
ip l set pc1r1 netns PC1
ip l set r1pc1 netns R1
ip netns exec PC1 ip l set pc1r1 up
ip netns exec PC1 ethtool -K pc1r1 tx off
ip netns exec R1 ip a add fd00::1:1/112 dev r1pc1
ip netns exec R1 ip l set r1pc1 up
ip netns exec R1 ethtool -K r1pc1 tx off
echo 'PC1 <-> R1 done'

# R1 <-> R2
ip l del r1r2 2>/dev/null
ip l add r1r2 type veth peer name r2r1
ip l set r1r2 netns R1
ip l set r2r1 netns R2
ip netns exec R1 ip a add fd00::3:1/112 dev r1r2
ip netns exec R1 ip l set r1r2 up
ip netns exec R1 ethtool -K r1r2 tx off
ip netns exec R2 ip l set r2r1 up
ip netns exec R2 ethtool -K r2r1 tx off
echo 'R1 <-> R2 done'

# enable IPv6 forwarding
ip netns exec R1 sysctl -w net.ipv6.conf.all.forwarding=1