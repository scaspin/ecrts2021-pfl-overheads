#!/bin/bash

# Generate the data
echo -e "\n// Running experiments"
./generate_data.sh

#Process the data
echo -e "\n// Aggregating results"
./generate_csvs_rw.sh

# Plot the results
echo -e "\n// Plotting results"
for i in 1 2 3 4 5 6 
do
	sleep 0.5 #make sure all csvs are valid
	echo -n "."
done
python3 plots_rw.py
echo "."

#Rename files
cp ../plots/read_overhead__cs=0.pdf ../paperplots/fig7d.pdf
cp ../plots/total_overhead_req=32_percwrites=5_cslen=0.pdf ../paperplots/fig7b.pdf
cp ../plots/total_overhead_req=32_percwrites=0_cslen=0.pdf ../paperplots/fig7a.pdf
cp ../plots/total_overhead_req=32_percwrites=50_cslen=0.pdf ../paperplots/fig7c.pdf
