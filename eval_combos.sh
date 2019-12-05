#!/bin/bash

TRIALS=100
REPS=3

./combinations.py NOYIELD PIN_THREAD USE_RDTSC NO_DOUBLE_ACCESS | while read line ; do
    for kwd in $line; do echo "#define $kwd" ; done > config.h
    make clean > /dev/null
    make > /dev/null
    echo -n $(echo $line | sed 's/ /+/g')\ 
    for j in `seq $REPS`; do
        echo -n $(for i in `seq $TRIALS`; do ./spy ; done | grep -c CORRECT)\ 
    done
    echo
done
