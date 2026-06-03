#!/usr/bin/env python3

import re
import sys

def main():
	if len(sys.argv) < 3:
		sys.exit(0)

	date_from = sys.argv[1]
	date_to   = sys.argv[2]

	pattern = re.compile(r"(\d{4}-\d{2}-\d{2})(.*) installed (.+):(.+).* " )
	
	for line in sys.stdin:
		line = line.rstrip()

		m = pattern.search(line)
		if m and m.group(1)>=date_from and m.group(1)<=date_to:
			print("%s %s"  % (m.group(1), m.group(3)))

if __name__ == "__main__":
	main()
