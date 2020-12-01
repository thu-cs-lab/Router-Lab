#!/usr/bin/env python3

with open('conf-part10.conf', 'w') as f:
    for i in range(1, 9):
        for j in range(0, 256):
            f.write('route 10.%d.%d.0/24 via "wlan0";\n' % (i, j))

