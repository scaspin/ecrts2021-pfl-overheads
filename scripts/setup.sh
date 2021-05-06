#!/bin/bash

totalcores=36
numsockets=2
taskcounts=( 2 4 6 8 10 12 14 16 18 20 22 24 26 28 30 32 34 36 )
corecounts=(${taskcounts[@]})

test=0
if [[ $test -eq 1 ]]
then
    echo "counts should be equal"
    echo ${taskcounts[@]}
    echo ${corecounts[@]}
fi

verbose=1
