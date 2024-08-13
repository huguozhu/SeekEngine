#!/usr/bin/env python
import os

root_path = os.path.dirname(os.path.abspath(os.path.dirname(__file__)))
scan_dirs = [
    "/common/dvf",
    "/common/meta",
    "/engine",
    "/tools",
    "/wrapper",
]

filepaths = []

for dir in scan_dirs:
    for root, _, files in os.walk(root_path+dir):
        for file in files:
            if file.endswith('.cpp') or file.endswith('.h') or file.endswith('.mm') or file.endswith('.m') or file.endswith('.hpp') or file.endswith('.txt'):
                filepaths.append(os.path.join(root, file))

for file in filepaths:
    print(file)
    with open(file, 'r') as f:
        lines = f.readlines()
    for line_i, line in enumerate(lines):
        lines[line_i] = line.expandtabs(4).rstrip() + '\n'
    with open(file, 'w') as f:
        f.writelines(lines)
