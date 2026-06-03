#!/usr/bin/env python3

import math

def main():
    a = 10.0
    b = 23.0
    
    c = math.sqrt(a ** 2 + b ** 2)
    
    print("A right triangle with sides", a, "and", b,
        "has hypotenuse of length", c)

    print("Again: Sides %.2f and %f, hypotenuse %.4f" % (a, b, c))

if __name__ == "__main__":
    main()
