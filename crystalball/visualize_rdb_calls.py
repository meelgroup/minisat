#!/home/arijit/anaconda3/bin/python3
# -*- coding: utf-8 -*-

import csv
import sys
import pandas as pd
import matplotlib as mpl
from matplotlib import pyplot as plt

def preprocess(outfile,csv_file):
    fieldnames = ['call', 'conflicts', 'dbsize']
    row = {}
    with open(csv_file, 'w') as csvfile, open(outfile, 'r') as outf:
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()
        for line in outf:
            if line.startswith("c [reduceDB]"):
                data = line[13:].split(" | ")
                row['call'] = int(data[0])
                row['conflicts'] = int(data[1])
                row['dbsize'] = int(data[4])
                writer.writerow(row)
def plot_data(csvfile):
    data = pd.read_csv(csvfile)
    plt.rcParams['figure.figsize'] = [8, 5]
    ax = data.plot(
    x='call',
    y='dbsize'
    )
    data.plot(
        x='call',
        y='conflicts',
        ax = ax.twinx(),
        color="indigo"
    )
    plt.show()


if __name__ == '__main__':
    outfile = sys.argv[1]
    csvfile = outfile[:-4]+".csv"
    preprocess(outfile,csvfile)
    plot_data(csvfile)
