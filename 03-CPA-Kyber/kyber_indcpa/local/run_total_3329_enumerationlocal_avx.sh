#!/usr/bin/env bash

TOTAL_PHYSICAL_CORES=`grep '^core id' /proc/cpuinfo | sort -u | wc -l`
TOTAL_LOGICAL_CORES=`grep '^core id' /proc/cpuinfo | wc -l`

# Load MSR module
sudo modprobe msr

# Setup
samples=10000	# 400 seconds
outer=10
date=`date +"%m%d-%H%M"`
thread=8 #FIXME

# Alert
echo "This script will take about $(($(($samples/1000+30))*$outer*1*3328/60/60/24)) days. Reduce 'outer' if you want a shorter run."

# Prepare
sudo rm -rf out
mkdir out
sudo rm -rf input.txt

# Only target index 0 
for guess_z in {1..3328}
do
# echo $guess_z 0 $thread >> input.txt
echo $guess_z 0 $TOTAL_LOGICAL_CORES >> input.txt
done
# Warm Up
stress-ng -q --cpu $TOTAL_LOGICAL_CORES -t 10m

# Run
sudo ./bin/driver_indcpa_avx ${samples} ${outer}
cp -r out data/out-${date}

# Unload MSR module
sudo modprobe -r msr
