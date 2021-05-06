#!/bin/bash

source setup.sh

folder="../csvs"
exe="../main_rw"

for cslength in 0
do
	for numresources in 32
        do
		for percwrites in 0 1 5 10 50 90 95 100
		do
			for alg in 1 2
			do
		    		for (( i = 0; i < ${#taskcounts[@]}; i++ ))
                    		do
                    			numtasks=${taskcounts[$i]}
                    			numcores=${corecounts[$i]}

                    			currenttime="`date +%H:%M:%S`"
                    			#echo "[$currenttime] n=$numtasks ($numcores cores), q=$numresources, cslength=$cslength, p = $percwrites, alg=$alg"
                    			echo -n "."
					timeout 2s $exe $numcores $numresources $numresources $cslength $percwrites $alg $folder/$alg/$numtasks-$numresources-$cslength-$percwrites-$alg.csv || $exe $numcores $numresources $numresources $cslength $percwrites $alg $folder/$alg/$numtasks-$numresources-$cslength-$percwrites-$alg.csv
				done
                    	done
	    	done
    	done
done

