#!/usr/bin/env python3

r = 1
with open('conf-part6-r%d.conf' % r, 'w') as f:
    for i in range(10, 256):
        f.write('route 192.168.%d.0/24 via "veth-R%d-PC%d";\n' % (i, r, r))

