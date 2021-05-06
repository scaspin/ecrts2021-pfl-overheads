import matplotlib
matplotlib.use('Agg')

import matplotlib.pyplot as plt
from typing import Dict, List, Tuple
from collections import defaultdict
import pandas as pd
from argparse import ArgumentParser

LINE_STYLE = ["s:", "^-", "o--", ".:", "+--", "x-.", "D:" ]

LINE_COLOR = ["#1f77b4", "#d95f02", "#1b9e77", "#d62728"] # for fig 7abc
LINE_COL = ["green", "blue", "purple", "red", "orange"] # for fig 7d

SCALING = 1
HEIGHT = 3
WIDTH = 6

IMAGES_DIR = "../plots"
CSV_DIR = "../csvs"

def plot_cores(res: int, pw: int, cslen: int):
    #fig = plt.figure()
    prefix = "req=" + str(res) + "_percwrites=" + str(pw) + "_cslen=" + str(cslen)
    df1 = pd.read_csv(f"{CSV_DIR}/rw_alg_1.csv")
    df2 = pd.read_csv(f"{CSV_DIR}/rw_alg_2.csv")
    
    #filter by appropriate values
    new_df1 = df1[df1['numresources']==res]
    new_df1 = new_df1[new_df1['percwrites']==pw]
    new_df1 = new_df1[new_df1['cslength']==cslen]
    new_df1 = new_df1.sort_values(by=['numcores'])

    new_df2 = df2[df2['numresources']==res]
    new_df2 = new_df2[new_df2['percwrites']==pw]
    new_df2 = new_df2[new_df2['cslength']==cslen]
    new_df2 = new_df2.sort_values(by=['numcores'])

    #plot overhead breakdown
    fig5 = plt.figure()
    plot_ob = fig5.add_subplot(1,1,1)
    plot_ob.grid(linestyle="dotted")
    plot_ob.spines["top"].set_visible(False)
    plot_ob.spines["right"].set_visible(False)

    # only plot reads if not 100% writes
    if (pw < 100):
        plot_ob.plot(
            new_df1["numcores"],
            new_df1["readoverhead"] / SCALING,
            LINE_STYLE[1],
            color=LINE_COLOR[1],
            label=f"PF-T reads",
        )

        plot_ob.plot(
            new_df2["numcores"],
            new_df2["readoverhead"] / SCALING,
            LINE_STYLE[2],
            color=LINE_COLOR[2],
            label=f"PF-L reads",
        )
    
    # only plot writes if not 100% reads
    if (pw > 0):
        plot_ob.plot(
            new_df1["numcores"],
            new_df1["writeoverhead"] / SCALING,
            LINE_STYLE[3],
            color=LINE_COLOR[3],
            label=f"PF-T writes",
        )
        plot_ob.plot(
            new_df2["numcores"],
            new_df2["writeoverhead"] / SCALING,
            LINE_STYLE[0],
            color=LINE_COLOR[0],
            label=f"PF-L writes",
        )
    
    plot_ob.set_xlabel("Number of cores")
    plot_ob.set_ylabel("Overhead (microseconds)")
    plot_ob.legend(loc='upper left',prop={'size': 12})
    plot_ob.set_yticks([0,0.5,1,1.5,2,2.5,3])
    plot_ob.set_ylim(0,3)

    fig5.set_size_inches(WIDTH, HEIGHT)
    plt.tight_layout()
    plt.savefig(f"{IMAGES_DIR}/total_overhead_{prefix}.pdf")

    plt.close('all')

def plotcs(cs: int):
    #fig = plt.figure()
    prefix =  "_cs=" + str(cs)
    df1 = pd.read_csv(f"{CSV_DIR}/rw_alg_2.csv")

    #filter by appropriate values
    new_df1 = df1[df1['cslength']==cs]
    new_df1 = new_df1.sort_values(by=['numcores'])

    fig4 = plt.figure()
    plot_o = fig4.add_subplot(1,1,1)
    plot_o.grid(linestyle="dotted")
    plot_o.spines["top"].set_visible(False)
    plot_o.spines["right"].set_visible(False)

    counter = 0
    for pw in [0,5,10,50,95]:
        df = new_df1[new_df1['percwrites']==pw]
        plot_o.plot(
            df["numcores"],
            df["readoverhead"] / SCALING,
            LINE_STYLE[counter],
            color=LINE_COL[counter],
            label=f"PF-L " + str(pw),
        )
        counter = counter + 1

    plot_o.set_xlabel("Number of cores")
    plot_o.set_ylabel("Overhead (microseconds)")
    plot_o.legend(loc='upper left', prop={'size': 12})
    plot_o.set_yticks([0,0.2,0.4,0.6,0.8])
    plot_o.set_ylim(0,0.8)

    fig4.set_size_inches(WIDTH, HEIGHT)
    plt.tight_layout()
    plt.savefig(f"{IMAGES_DIR}/read_overhead_{prefix}.pdf")
    plt.close('all')

if __name__ == "__main__":
    for percwrites in [0,5,50]:
        plot_cores(32, percwrites, 0)
    plotcs(0)
