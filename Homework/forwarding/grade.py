#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import re
import sys
import os
import json
import subprocess
import time
from os.path import isfile, join
import random
import string
import signal
import glob
import traceback

prefix = 'forwarding'
exe = prefix
if len(sys.argv) > 1:
    exe = sys.argv[1]

try:
    import pyshark
except Exception:
    print('Install pyshark (pip3 install pyshark) first!')
    sys.exit(1)

def write_grade(grade, total):
    data = {}
    data['grade'] = grade
    if os.isatty(1):
        print('Passed: {}/{}'.format(grade, total))
    else:
        print(json.dumps(data))

    sys.exit(0)


if __name__ == '__main__':

    if sys.version_info[0] != 3:
        print("Plz use python3")
        sys.exit()

    if os.isatty(1):
        print('Removing all output files')
    os.system('rm -f data/{}user*.out'.format(prefix))

    total = len(glob.glob("data/{}_input*.pcap".format(prefix)))

    grade = 0

    for i in range(1, total+1):
        in_file = "data/{}_input{}.pcap".format(prefix, i)
        out_file = "data/{}_user{}.out".format(prefix, i)
        ans_file = "data/{}_output{}.out".format(prefix, i)

        if os.isatty(1):
            print('Running \'./{} < {} > {}\''.format(exe, in_file, out_file))
        p = subprocess.Popen(['./{}'.format(exe)], stdout=open(out_file, 'w'), stdin=open(in_file, 'r'))
        start_time = time.time()

        while p.poll() is None:
            if time.time() - start_time > 1:
                p.kill()

        try:
            out = [line.strip() for line in open(out_file, 'r').readlines() if line.strip()]
            ans = [line.strip() for line in open(ans_file, 'r').readlines() if line.strip()]
                
            if out == ans:
                grade += 1
            elif os.isatty(1):
                limit = 1
                count = 0
                reader = pyshark.FileCapture(in_file)
                packets = list(reader)
                print('Wrong Answer (showing only first {} packets):'.format(limit))
                for i in range(len(ans)):
                    if i >= len(out) or out[i] != ans[i]:
                        count += 1
                        print('Answer is wrong for packet #{}: {}'.format(i, packets[i]['ip']))
                        if count == limit:
                            break
                print('Diff: ')
                os.system('diff -u {} {} | head -n 10'.format(out_file, ans_file))
                reader.close()
        except Exception:
            if os.isatty(1):
                print('Unexpected exception caught:')
                traceback.print_exc()

    write_grade(grade, total)

