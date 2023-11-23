#!/bin/bash

ip netns exec R3 bird -c bird-r3-v2-ripng.conf -d -s bird-r3-ripng.ctl
