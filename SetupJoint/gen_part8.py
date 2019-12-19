#!/usr/bin/env python3

import json

with open('as4538_prefixes', 'r') as prefix:
    data = json.load(prefix)['data']['ipv4_prefixes']
    r = 1
    with open('conf-part8-r%d.conf' % r, 'w') as f:
        for item in data:
            f.write('route %s via "veth-R%d-PC%d";\n' % (item['prefix'], r, r))

