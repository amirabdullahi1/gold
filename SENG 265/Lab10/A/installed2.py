#!/usr/bin/env python3

import re
import fileinput


def main():
    line_number = 0

    for line in fileinput.input():
        line = line.rstrip()
        line_number += 1

        m = re.search(r" installed (.+) .*", line)
        if m:
            print("%d: %s" % (line_number, m.group(1)))

if __name__ == "__main__":
    main()
