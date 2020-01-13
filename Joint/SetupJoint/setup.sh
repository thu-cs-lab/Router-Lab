#!/bin/bash
set -v

killall bird
ip netns del PC1
ip netns del PC2
ip netns del PC3
ip netns del PC4
ip netns del R1
ip netns del R2
ip netns del R3
ip netns del R4

ip link del veth-PC1-R1
ip link del veth-R1-PC1
ip link del veth-PC2-R2
ip link del veth-R2-PC2
ip link del veth-PC3-R3
ip link del veth-R3-PC3
ip link del veth-PC4-R4
ip link del veth-R4-PC4

ip netns add PC1
ip netns add PC2
ip netns add PC3
ip netns add PC4
ip netns add R1
ip netns add R2
ip netns add R3
ip netns add R4

ip l add veth-PC1-R1 type veth peer name veth-R1-PC1
ip l add veth-PC2-R2 type veth peer name veth-R2-PC2
ip l add veth-PC3-R3 type veth peer name veth-R3-PC3
ip l add veth-PC4-R4 type veth peer name veth-R4-PC4

ip l set veth-PC1-R1 netns PC1
ip l set veth-R1-PC1 netns R1
ip l set veth-PC2-R2 netns PC2
ip l set veth-R2-PC2 netns R2
ip l set veth-PC3-R3 netns PC3
ip l set veth-R3-PC3 netns R3
ip l set veth-PC4-R4 netns PC4
ip l set veth-R4-PC4 netns R4

ip netns exec PC1 ip a add 192.168.5.3/24 dev veth-PC1-R1
ip netns exec PC1 ip l set veth-PC1-R1 up
ip netns exec PC1 ip r add default via 192.168.5.2
ip netns exec PC2 ip a add 192.168.6.3/24 dev veth-PC2-R2
ip netns exec PC2 ip l set veth-PC2-R2 up
ip netns exec PC2 ip r add default via 192.168.6.2
ip netns exec PC3 ip a add 192.168.7.3/24 dev veth-PC3-R3
ip netns exec PC3 ip l set veth-PC3-R3 up
ip netns exec PC3 ip r add default via 192.168.7.2
ip netns exec PC4 ip a add 192.168.8.3/24 dev veth-PC4-R4
ip netns exec PC4 ip l set veth-PC4-R4 up
ip netns exec PC4 ip r add default via 192.168.8.2

ip netns exec R1 ip a add 192.168.5.2/24 dev veth-R1-PC1
ip netns exec R1 ip l set veth-R1-PC1 up
ip netns exec R2 ip a add 192.168.6.2/24 dev veth-R2-PC2
ip netns exec R2 ip l set veth-R2-PC2 up
ip netns exec R3 ip a add 192.168.7.2/24 dev veth-R3-PC3
ip netns exec R3 ip l set veth-R3-PC3 up
ip netns exec R4 ip a add 192.168.8.2/24 dev veth-R4-PC4
ip netns exec R4 ip l set veth-R4-PC4 up

ip l set eno1 netns R1
ip l set eno2 netns R2
ip l set eno3 netns R3
ip l set eno4 netns R4

# might fail, try again
ip l set eno1 netns R1
ip l set eno2 netns R2
ip l set eno3 netns R3
ip l set eno4 netns R4

ip netns exec R1 ip a add 192.168.0.2/24 dev eno1
ip netns exec R1 ip l set eno1 up
ip netns exec R2 ip a add 192.168.1.2/24 dev eno2
ip netns exec R2 ip l set eno2 up
ip netns exec R3 ip a add 192.168.2.2/24 dev eno3
ip netns exec R3 ip l set eno3 up
ip netns exec R4 ip a add 192.168.3.2/24 dev eno4
ip netns exec R4 ip l set eno4 up

ip netns exec R1 sysctl -w net.ipv4.ip_forward=1
ip netns exec R2 sysctl -w net.ipv4.ip_forward=1
ip netns exec R3 sysctl -w net.ipv4.ip_forward=1
ip netns exec R4 sysctl -w net.ipv4.ip_forward=1

ip netns exec R1 bird -d -c bird1.conf -P bird1.pid -s bird1.socket 2> bird1.log &
ip netns exec R2 bird -d -c bird2.conf -P bird2.pid -s bird2.socket 2> bird2.log &
ip netns exec R3 bird -d -c bird3.conf -P bird3.pid -s bird3.socket 2> bird3.log &
ip netns exec R4 bird -d -c bird4.conf -P bird4.pid -s bird4.socket 2> bird4.log &
 
tmux kill-session
tmux new-session -d 'ip netns exec PC1 fish'
tmux split-window -v 'ip netns exec PC3 fish'
tmux split-window -h 'ip netns exec PC4 fish'
tmux split-window -v 'ip netns exec R4 fish'
tmux select-pane -U
tmux select-pane -U
tmux split-window -h 'ip netns exec PC2 fish'
tmux split-window -v 'ip netns exec R2 fish'
tmux select-pane -L
tmux split-window -v 'ip netns exec R1 fish'
tmux select-pane -D
tmux split-window -v 'ip netns exec R3 fish'
tmux setw -g mouse on
tmux attach-session
