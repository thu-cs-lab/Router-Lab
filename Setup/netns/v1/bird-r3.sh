#!/bin/bash

ip netns exec R3 bird6 -c bird-r3.conf -d -s bird-r3.ctl
