#!/usr/bin/env bash
# Reproduce paper figure: measure near-s and random pairs.
# For full hint pair search, use hint_pair_search/run.sh

TOTAL_PHYSICAL_CORES=`grep '^core id' /proc/cpuinfo | sort -u | wc -l`
TOTAL_LOGICAL_CORES=`grep '^core id' /proc/cpuinfo | wc -l`

# Load MSR module
sudo modprobe msr

# Setup
samples=8000	# 120 seconds
outer=10
date=`date +"%m%d-%H%M"`
thread=8 #FIXME

# Alert
echo "This script will take about $(($(($samples/1000))*$outer*100/60)) minitues. Reduce 'outer' if you want a shorter run."

# Prepare
sudo rm -rf data/tmp
mkdir -p data/tmp
sudo rm -rf input.txt

# RIGHT S 2246/3165
# Only target index 0 
 
# 98 random other $z$ +2246/3165
for guess_z in 1 30 75 100 185 251 374 388 465 494 516 530 531 565 626 638 651 725 756 811 845 888 903 918 934 941 987 1018 1082 1141 1207 1220 1221 1251 1279 1291 1297 1300 1301 1327 1339 1345 1357 1399 1403 1423 1475 1522 1631 1642 1646 1676 1694 1712 1723 1727 1737 2246 1778 1784 1793 1802 1805 1814 1820 1885 1893 1915 1986 2008 2013 2034 2099 2202 2240 2248 2264 2294 2303 2374 2440 2454 2480 2498 2549 2604 2612 2661 2795 2886 2901 2904 3165 2928 2968 2997 3010 3077 3174 3268
do
echo 1 $guess_z 0 $thread >> input.txt
done


# #input 
# #2630,1490
# for guess_z1 in 2630
# do
# echo $guess_z1 $((guess_z1*2246%3329)) 0 $thread >> input.txt
# done


# Warm Up
stress-ng -q --cpu $TOTAL_LOGICAL_CORES -t 10m

# Run
sudo ./bin/driver_indcpa_avx ${samples} ${outer}

# Change ownership to current user
sudo chown -R $USER:$USER data/tmp

# Archive results
cp -r data/tmp data/out-${date}

# Unload MSR module
sudo modprobe -r msr
