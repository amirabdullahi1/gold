#!/usr/bin/env python3

import re
import fileinput


def main():
    line_number = 0

    pattern = re.compile(r" installed ((.+):(.+)) .*")

    for line in fileinput.input():
        line = line.rstrip()
        line_number += 1

        m = pattern.search(line)
        if m:
            print("%d: %s" % (line_number, m.group(2)))

if __name__ == "__main__":
    main()
