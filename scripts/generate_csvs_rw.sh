#!/bin/bash

infolder="../csvs"
outfolder="../csvs"

for alg in 1 2
do
   echo -n "."
   python ../src/to_csv_rw.py $infolder/$alg/ $outfolder/rw_alg_$alg.csv
   echo -n "."
done

