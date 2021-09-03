#!/usr/bin/env python3

with open('conf-part9.conf', 'w') as f:
    f.write('# assuming lo exists and is up\n')
    for i in range(10, 256):
        f.write('route 192.168.%d.0/24 via "lo";\n' % (i))


with open('conf-part10.conf', 'w') as f:
    f.write('# assuming lo exists and is up\n')
    for i in range(1, 9):
        for j in range(0, 256):
            f.write('route 10.%d.%d.0/24 via "lo";\n' % (i, j))

