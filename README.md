# ecrts2021-pfl-overheads
Artifact evaluation code for overhead evaluation in ECRTS'21 "Light-Reading" 

Text (and code) in part was borrowed from Nemitz, Amert, and Anderson's 
“Using Lock Servers to Scale Real-Time Locking Protocols: Chasing Ever-Increasing Core Counts” 
which can be found here: https://jamesanderson.web.unc.edu/papers/

Some code in /include was taken from https://www.cs.unc.edu/~bbb/diss/

## Configurations

The following are required to compile the code to run experiments:
* gcc (we ran our experiments on ubuntu 14.04.4 with GCC 4.8.4,
       and tested the artifact evaluation on package also on a machine
       running Debian 3.10.7 with gcc 5.2.1)

The following are required to visualize the results of experiments:
* Python 2.7, Python3
* matplotlib/numpy Python libraries

Our experiments are designed to run on a machine with multiple *sockets*,
each with multiple cores.  For example, we ran the experiments in the paper
on a machine with two sockets, each with 18 physical cores.

The next steps will help you configure the experimental setup for your machine.
For these instructions, we'll assume that $aedir is the path to our
artifact evaluation directory.

1. Determine the number of sockets on your machine.  Note that we tested
   our artifact evaluation packet on two different machines, one with two
   sockets and one with four, but we used only two sockets on each.  The
   code would require some extra reconfiguration to handle more than two
   sockets, but there is nothing inherently preventing this.    

    // Example (2 sockets):  
    `lscpu | grep "Socket(s):"`    
    Socket(s):             2  

2. Determine the number of physical cores per socket.    

    // Example (72 cores, 2 virtual cores per core; we will use 0-35):  
    `lscpu | grep "CPU(s) list:"`  
    On-line CPU(s) list:   0-71  

    `cat /sys/devices/system/cpu/cpu*/topology/thread_siblings_list | sort --unique --numeric-sort`  
    0,36    
    1,37    
    ...    
    34,70    
    35,71    

3. Determine layout of physical core numbers on the sockets.    

    // Example (cores alternate on each socket):    
    % cat /proc/cpuinfo | grep "physical id"    
    physical id     : 0    
    physical id     : 1    
    ...    
    physical id     : 0    
    physical id     : 1    

4. Determine the processor speed of the machine.    

    // Example:    
    % lscpu | grep "CPU MHz:"    
    CPU MHz:               2297.579    
    
5. Set the variables with "TODO"s in $aedir/include/setup.h.    

    // Example (36 physical cores on 2 sockets, alternating cores on each socket):    
    #define TOTAL_CORES 36    
    #define NUM_SOCKETS 2    
    
    #define PROC_SPEED 2297.579    

    static int CPU_MAP[TOTAL_CORES} = {
        // Socket 0
        0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,
        // Socket 1
        1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35
    };

6. Set the processor speed in $aedir/src/to_csv_rw.py.    

    // Example:    
    procSpeed = 2297.579    
    
7. Set up the experiments' core counts with "TODO"s in $aedir/scripts/setup.sh.    

    // Example (same as above, 36 physical cores on 2 sockets...):    
    totalcores=36    
    numsockets=2    
    taskcounts=( 2 4 6 8 10 12 14 16 18 20 22 24 26 28 30 32 34 36 )    


## Compilation

`cd` into the directory and build all code with:    
```
    % cd $aedir
    % make all
```

Note: there might be warnings, but you should not get any error messages.    

## Usage

To run the experiments, `cd` into the scripts directory, and run
the file run_artifact_evaluation.sh:
```
     cd scripts
     ./run_artifact_evaluation.sh
```

The results will be aggregated into $aedir/csvs/alg#/     

The processed files will be aggregated per-algorithm into a CSV file
in $aedir/csvs/rw_alg_#.csv . This may take a couple of seconds or 
minutes for larger experiments.

All plots will be placed in $aedir/plots/, and are labeled
by write precentage.

All plots in the paper are copied and renamed in $aedit/paperplots/
and labeled by figure and inset.

`$aedir/scripts/cleanup.sh` will delete all csv files and plots.

All scripts in the scripts directory need to be run from that directory.

Note: You can set TRIALS in $aedir/include/setup.h to be 10000 to get more
accurate data, but then the experiments will take longer to run.  Keep it
at 1000 to validate running the code, but it is advised to use 10000 to
validate the results in the paper. You may need to change the `#define BUFSIZE (TRIALS / 10)`
in $aedir/include/setup to divide by 100 instead.


## Repeatable Experiments          

The instructions above should reproduce the graphs for figure 7
into $aedir/paperplots/ . All additional plots can be seen in $aedir/plots
For ease of comparison, directory "sampleplots" has plots reproduced by these 
experiments on the same machine as used in the paper.
