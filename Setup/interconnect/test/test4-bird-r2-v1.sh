#!/bin/sh
# Test 4
# Check OSPF neighbor state
sudo birdc -s ../setup/bird-v1/bird-r2-ospf.ctl show ospf neighbor
