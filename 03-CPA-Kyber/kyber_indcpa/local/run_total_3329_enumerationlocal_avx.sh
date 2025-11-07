#!/usr/bin/env bash

TOTAL_PHYSICAL_CORES=`grep '^core id' /proc/cpuinfo | sort -u | wc -l`
TOTAL_LOGICAL_CORES=`grep '^core id' /proc/cpuinfo | wc -l`

# Load MSR module
sudo modprobe msr

# Setup
samples=800	# 32 seconds
outer=1
date=`date +"%m%d-%H%M"`
thread=8 #FIXME

# Alert
echo "This script will take about $(($(($samples/1000+30))*$outer*1*3328/60/60/24)) days. Reduce 'outer' if you want a shorter run."

# Prepare
sudo rm -rf data/tmp
mkdir -p data/tmp
sudo rm -rf input.txt

# Only target index 0 - test only first 200 z values for quick test
for guess_z in {1..200}
do
# echo $guess_z 0 $thread >> input.txt
echo $guess_z 0 $TOTAL_LOGICAL_CORES >> input.txt
done
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
