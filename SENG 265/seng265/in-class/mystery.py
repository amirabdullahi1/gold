#!/usr/bin/python 
import sys 

def main():
	(aaa, bbb, ccc) = (1, 0, {}) 
	
	ddd = sys.stdin.readlines() 

	while (ddd != []): 
		eee = ddd[0]
		fff = eee.strip().split() 
		for ggg in fff: 
			#if ggg in ccc.keys(): 
			#	ccc[ggg] = ccc[ggg] + ", " + ("%d" % aaa) 
			#else: 
			ccc[ggg] = ("%d" % aaa) 
			print ccc.keys()  
		aaa = aaa + 1
		ddd = ddd[1:] #shifts start of listto second element

	hhh = ccc.keys() 
	print hhh
	hhh.sort() #alphabetizes keys 
	print hhh 

	hhh = hhh[0:5] 

	for h in hhh: 
		print h, ":" , ccc[h] 

if __name__ == "__main__":
	main()
