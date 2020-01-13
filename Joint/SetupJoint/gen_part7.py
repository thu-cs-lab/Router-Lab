#!/usr/bin/env python3

for r in range(1, 5):
    with open('conf-part7-r%d.conf' % r, 'w') as f:
        for i in range(1, 3):
            for j in range(0, 256):
                f.write('route 10.%d.%d.0/24 via "veth-R%d-PC%d";\n' % (i+r*2-2, j, r, r))

