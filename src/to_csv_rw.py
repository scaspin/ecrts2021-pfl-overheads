import sys

from numpy import percentile
from numpy import loadtxt
import glob
import os
import csv
import re

# TODO: Set processor speed in MHz
procSpeed = 2297.579

def main():
    if len(sys.argv) != 3:
        print >> sys.stderr, "Usage: to_csv.py directory out.csv"
        sys.exit(1)

    regexp = re.compile('.*/(.*)-(.*)-(.*)-(.*)-(.*).csv') # numcores-numresources-cslength-perc_writes-ALG
    
    with open(sys.argv[2], 'wb') as outfile:
        fieldnames = ['numcores', 'numresources', 'numrequested', 'percwrites', 'cslength',
                      'readlock','readunlock', 'readblocking', 'writelock', 'writeunlock', 'writeblocking', 'readoverhead', 'writeoverhead', 'overhead']

        writer = csv.DictWriter(outfile, fieldnames = fieldnames)
        writer.writeheader()

        for file in glob.glob(sys.argv[1]+"/*.csv"):
            #print file
            row = {}
            m = regexp.match(file)
            row['numcores']     = m.group(1)
            row['numresources'] = m.group(2)
            row['numrequested'] = row['numresources']
            row['cslength']   = m.group(3)
            row['percwrites']  = m.group(4)

            try:
                data = loadtxt(file, skiprows=1, delimiter=",")
            except:
                print "Failed to load file:", file

            row['readlock'] = percentile(data[:,6]/procSpeed, 99)
            row['readunlock'] = percentile(data[:,7]/procSpeed, 99)
            row['readblocking'] = percentile(data[:,8]/procSpeed, 99)
            row['writelock'] = percentile(data[:,9]/procSpeed, 99)
            row['writeunlock'] = percentile(data[:,10]/procSpeed, 99)
            row['writeblocking'] = percentile(data[:,11]/procSpeed, 99)
            row['readoverhead'] = percentile((data[:,6]+data[:,7])/procSpeed, 99)
            row['writeoverhead'] = percentile((data[:,9]+data[:,10])/procSpeed, 99)
            row['overhead'] = percentile((data[:,6]+data[:,7]+data[:,9]+data[:,10])/procSpeed, 99)

            """
            #if want an approximation of median
            #if wanted to calculate near-average values
            row['readlock'] = percentile(data[:,6]/procSpeed, 50)
            row['readunlock'] = percentile(data[:,7]/procSpeed, 50)
            row['readblocking'] = percentile(data[:,8]/procSpeed, 50)
            row['writelock'] = percentile(data[:,9]/procSpeed, 50)
            row['writeunlock'] = percentile(data[:,10]/procSpeed, 50)
            row['writeblocking'] = percentile(data[:,11]/procSpeed, 50)
            row['overhead'] = percentile((data[:,6]+data[:,7]+data[:,9]+data[:,10])/procSpeed, 50)
            
            """
            writer.writerow(row)

if __name__ == '__main__':
    main()

