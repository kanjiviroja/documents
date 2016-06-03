#!/usr/bin/env python
# load system and math module:
import sys, math
# extract the 1st command-line argument:
r = float(sys.argv[1])
s = math.sin(r)
print "Hello, World! sin(" + str(r) + ")=" + str(s)
