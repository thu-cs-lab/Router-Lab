#!/bin/bash
set -v

birdc -s bird1.socket enable part6
birdc -s bird1.socket disable part7
birdc -s bird1.socket disable part8

birdc -s bird2.socket enable part6
birdc -s bird2.socket disable part7
birdc -s bird2.socket disable part8

birdc -s bird3.socket enable part6
birdc -s bird3.socket disable part7
birdc -s bird3.socket disable part8

birdc -s bird4.socket enable part6
birdc -s bird4.socket disable part7
birdc -s bird4.socket disable part8
