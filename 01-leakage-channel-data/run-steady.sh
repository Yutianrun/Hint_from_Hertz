#!/usr/bin/env bash

TOTAL_PHYSICAL_CORES=`grep '^core id' /proc/cpuinfo | sort -u | wc -l`
TOTAL_LOGICAL_CORES=`grep '^core id' /proc/cpuinfo | wc -l`

# Load MSR module
sudo modprobe msr

# Setup
samples=100		# 1 second
outer=10 # 3 reps
num_thread=$TOTAL_LOGICAL_CORES
date=`date +"%m%d-%H%M"`

# Alert
echo "This script will take about $(((10)*$outer*3/60+10)) minutes. Reduce 'outer' if you want a shorter run."

# Warmup
stress-ng -q --cpu $TOTAL_LOGICAL_CORES -t 1m

# Run
sudo rm -rf out
mkdir out
sudo rm -rf input.txt

for selector in 48 32 16; do
	echo $selector >> input.txt
done

sudo ./bin/driver-steady ${num_thread} ${samples} ${outer}
cp -r out data/out-steady-${date}

# Unload MSR module
sudo modprobe -r msr
