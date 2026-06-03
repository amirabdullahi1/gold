#!/usr/bin/env python3
import math
import sys

def main():
	args=sys.argv
	# Write your code here. Refer to the hints provided in the lab PDF.
	if len(args)!=2:
		print("wrong number of args")
		return
	input=args[1].split(',')
	#print(input)			
	
	for w in range(len(input)): 
		print(input[w])

if __name__ == "__main__":
	main()
