#!/usr/bin/env bash

TOTAL_PHYSICAL_CORES=`grep '^core id' /proc/cpuinfo | sort -u | wc -l`
TOTAL_LOGICAL_CORES=`grep '^core id' /proc/cpuinfo | wc -l`

# Load MSR module
sudo modprobe msr

# Setup
samples=20000		# 10 seconds
outer=10			# 30 reps
num_thread=$TOTAL_LOGICAL_CORES
date=`date +"%m%d-%H%M"`

# Alert
echo "This script will take about $(($(($samples/1000+20))*$outer*2/60+10)) minitues. Reduce 'outer' if you want a shorter run."

# Warmup
stress-ng -q --cpu $TOTAL_LOGICAL_CORES -t 10m

# Run
sudo rm -rf out
mkdir out
sudo rm -rf input.txt

sudo ./bin/driver_avx ${num_thread} ${samples} ${outer}
cp -r out data/out-avx-${date}

# Unload MSR module
sudo modprobe -r msr