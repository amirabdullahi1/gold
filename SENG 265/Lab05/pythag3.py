#!/usr/bin/env python3
import math
import sys

def pythag(a,b): 
	return math.sqrt(a ** 2 + b ** 2)

def main():
 
    # parse the command-line arguments
    args = sys.argv
 
	# prin  an error and quit if there aren't the right number
    if len(args)!=3:
        print("wrong number of arguments")
        return
    else:
        print("this is the first number entered ", args[1])
        print("this is the second number entered ", args[2])	
	# convert the command line arguments from strings to floats
    a=float(args[1])
    b=float(args[2])
    print("Sides ", a, " and ", b, ", hypotenuse ", end="", sep="")
    print("%.4f" % pythag(a,b))

if __name__ == "__main__":

    main()
