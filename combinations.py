#!/usr/bin/env python

import sys
from itertools import chain, combinations

def powerset(iterable):
    xs = list(iterable)
    return chain.from_iterable(combinations(xs,n) for n in range(len(xs)+1))

for combo in powerset(sys.argv[1:]):
    print " ".join(combo)
