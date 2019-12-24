#!/bin/bash
echo "Enable part7 and disable part{8,9}"
set -v

birdc restart all
birdc enable part7
birdc disable part8
birdc disable part9