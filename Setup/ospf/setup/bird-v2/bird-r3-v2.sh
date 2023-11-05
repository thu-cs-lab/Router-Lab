#!/bin/bash

ip netns exec R3 bird -c bird-r3-v2.conf -d -s bird-r3.ctl
