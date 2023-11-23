#!/bin/bash

ip netns exec R1 bird -c bird-r1-v2.conf -d -s bird-r1.ctl
