#!/bin/sh
# Start custom tftp server on pc2
cd ../server && ip netns exec PC2 ../../../Homework/tftp/pc2/server
