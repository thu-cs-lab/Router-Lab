#!/bin/bash
echo "Enable part8 and disable part{7,9}"
set -v

systemctl start bird
birdc restart all
birdc enable part8
birdc disable part7
birdc disable part9