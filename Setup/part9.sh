#!/bin/bash
echo "Enable part9 and disable part{7,8}"
set -v

birdc restart all
birdc enable part9
birdc disable part7
birdc disable part8