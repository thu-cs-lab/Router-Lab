#!/bin/bash

ip netns exec R1 bird6 -c bird-r1.conf -d -s bird-r1.ctl
