#!/bin/bash
# Setup netns for individual judge
# IPv6 forwarding is enabled for R1 & R3

# netns
for ns in PC1 R1 R2 R3 PC2
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
ip netns exec PC1 ip a add fd00::1:2/112 dev pc1r1
ip netns exec PC1 ip l set pc1r1 up
ip netns exec PC1 ip -6 r add default via fd00::1:1
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

# R2 <-> R3
ip l del r2r3 2>/dev/null
ip l add r2r3 type veth peer name r3r2
ip l set r2r3 netns R2
ip l set r3r2 netns R3
ip netns exec R3 ip a add fd00::4:2/112 dev r3r2
ip netns exec R3 ip l set r3r2 up
ip netns exec R3 ethtool -K r3r2 tx off
ip netns exec R2 ip l set r2r3 up
ip netns exec R2 ethtool -K r2r3 tx off
echo 'R2 <-> R3 done'

# R3 <-> PC2
ip l del r3pc2 2>/dev/null
ip l add r3pc2 type veth peer name pc2r3
ip l set r3pc2 netns R3
ip l set pc2r3 netns PC2
ip netns exec PC2 ip a add fd00::5:1/112 dev pc2r3
ip netns exec PC2 ip l set pc2r3 up
ip netns exec PC2 ip -6 r add default via fd00::5:2
ip netns exec PC2 ethtool -K pc2r3 tx off
ip netns exec R3 ip a add fd00::5:2/112 dev r3pc2
ip netns exec R3 ip l set r3pc2 up
ip netns exec R3 ethtool -K r3pc2 tx off
echo 'R3 <-> PC2 done'

# enable IPv6 forwarding
ip netns exec R1 sysctl -w net.ipv6.conf.all.forwarding=1
ip netns exec R3 sysctl -w net.ipv6.conf.all.forwarding=1
