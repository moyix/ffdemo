#!/bin/bash

TRIALS=100
REPS=3

iters=8
while [ $iters -lt $[ 64*1024*1024 ] ]; do
    cat > config.h <<EOF
#define NO_DOUBLE_ACCESS
#define NUMBER_OF_TRIALS ($iters)
EOF

    make clean > /dev/null
    make > /dev/null
    echo -n $iters\ 
    for j in `seq $REPS`; do
        echo -n $(for i in `seq $TRIALS`; do ./spy ; done | grep -c CORRECT)\ 
    done
    echo
    iters=$[ $iters * 2 ]
done
