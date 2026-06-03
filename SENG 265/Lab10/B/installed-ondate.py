#!/usr/bin/env python3

import re
import sys

def main():
	if len(sys.argv) < 2:
		sys.exit(0)
	
	line_number = 0
	input_date = sys.argv[1] 
		
	pattern = re.compile(r"(\d{4}-\d{2}-\d{2})(.*) installed (.+):(.+).* " )
	 
	for line in sys.stdin:
		line = line.rstrip()
		line_number += 1

		m = pattern.search(line)
		if m and m.group(1)==input_date:
			print("%s %s"  % (m.group(1), m.group(3)))

if __name__ == "__main__":
	main()
