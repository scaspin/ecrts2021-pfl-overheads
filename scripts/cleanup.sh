#!/bin/bash

echo -e "\n//Deleting CSV files"
rm ../csvs/1/*.csv
rm ../csvs/2/*.csv
rm ../csvs/*csv

echo -e "//Deleting extra plots"
rm ../plots/*.pdf

echo -e "//Deleting paper plots"
rm ../paperplots/*.pdf
