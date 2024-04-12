import warnings
warnings.simplefilter(action='ignore', category=FutureWarning)

import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

import sys

plt.rcParams['font.family'] = "Noto Sans CJK JP"
filename = sys.argv[1]

my_file = open(filename, 'r')

time={}
time0 = []
time1 = []
time_value = 0
for line in my_file:
    if("guess_z" in line):
        time_value = int(line.split()[1])
    if("kyber took about" in line):
        curr_time = line.split()[3]
        curr_time_float = float(curr_time)*1000

        time.setdefault(time_value,[])
        time[time_value].append(curr_time_float)


# filter_time0 = []
# filter_time1 = []
# time0_mean = np.mean(time0)
# time1_mean = np.mean(time1)
# time0_std = np.std(time0)
# time1_std = np.std(time1)
filters={}

for guess_z, time_i in time.items():
    # time_i = [i for i in time_i if i > 9600]
    time_std = np.std(time_i)
    time_mean =  np.mean(time_i)
    for sample in time_i:
        if abs(sample - time_mean) <= 10* time_std:
            filters.setdefault(guess_z,[])
            filters[guess_z].append(sample)


fig, ax = plt.subplots()
sns.set_theme()
# sns.set(rc={'figure.figsize':(8,4)})
# sns.set_style("darkgrid")
time_1_label = r"$miss trigger$"
from matplotlib import rcParams

# figure size in inches
plt.figure(figsize=(8,3.5))


sns.set(style="whitegrid", font_scale=1.2, rc={"font.family": "sans-serif", "font.sans-serif": ["Noto Sans CJK JP"]})
plt.xlabel("运行时间(ms)")
plt.ylabel("概率密度")
plt.tight_layout()
time_0_label = r"$hit trigger$"
# r"$p_0:[C_1,C_2,\cdots]$",r"$p_1:[C_3,0,\cdots]$"

# lable_category = [r'$\hat{r}_0:[C_0,C_1,\cdots]$',r"$\hat{r}_1:[C_2,0,\cdots]$",r"error",r"$\hat{r}_0\!:\![C_0,\cdots,0,C_1]\!$",r"$\hat{r}_0\!:\![C_0;0;,\cdots,C_2,0]\!$",]
for guess_z , filter_value in sorted(filters.items(),reverse=True): 
    if guess_z != 1760:
        C0_or_not = r'$\hat{r}_0:[C_0,C_1,\cdots]$'
    else:
        C0_or_not = r"$\hat{r}_1:[C_2,0,\cdots]$"
    

    sns.histplot(filter_value, bins=250, label=C0_or_not, alpha = 0.3, edgecolor='k', linewidth=0.8,kde=True,stat = 'density')

    print("%15s (%6d samples): %.15f +- %.15f J" % (C0_or_not, len(filter_value), np.mean(filter_value), np.std(filter_value)))

# plt.xlim(0.0605,0.0655)
# plt.xlim(9200,10000)
plt.legend()
sns.despine()
plt.savefig("./plot/local-time-plot.pdf")
