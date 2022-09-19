#!/usr/bin/env python3
import json

with open('conf-part9.conf', 'w') as f:
    f.write('# assuming lo exists and is up\n')
    for i in range(10, 256):
        f.write('route fd00::%x:0/112 via "lo";\n' % (i))


with open('conf-part10.conf', 'w') as f:
    f.write('# assuming lo exists and is up\n')
    for i in range(1, 9):
        for j in range(0, 256):
            f.write('route fd00::%x:%x:0/112 via "lo";\n' % (i, j))


# https://api.bgpview.io/asn/9808/prefixes
data = json.load(open('as9808_prefixes'))['data']['ipv6_prefixes']
with open('conf-part11.conf', 'w') as f:
    f.write('# assuming lo exists and is up\n')
    for entry in data:
        f.write('route %s via "lo";\n' % entry['prefix'])
