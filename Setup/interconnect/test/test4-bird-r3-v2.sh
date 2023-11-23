#!/bin/sh
# Test 4
# Check OSPF neighbor state
sudo birdc -s ../setup/bird-v2/bird-r3-ospf.ctl show ospf neighbor
