#!/bin/sh
# Test 3
# Check OSPF neighbor state
sudo birdc -s ../setup/bird-v1/bird-r1.ctl show ospf neighbor
sudo birdc -s ../setup/bird-v1/bird-r3.ctl show ospf neighbor