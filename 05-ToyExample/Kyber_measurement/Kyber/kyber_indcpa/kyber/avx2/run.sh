#!/usr/bin/env bash

TOTAL_PHYSICAL_CORES=`grep '^core id' /proc/cpuinfo | sort -u | wc -l`
TOTAL_LOGICAL_CORES=`grep '^core id' /proc/cpuinfo | wc -l`

# Load MSR module
# sudo modprobe msr


# for i in {1,151,901,1501,1601,1851,2101,2551,3201,3651,3801,3901,4301,4501,5451,5501,6301,6701,7051,7251}

# do

# # Setup
# samples=20000	# 20 seconds
# outer=10
# date=`date +"%m%d-%H%M"`
# thread=8

# echo "This script will take about $(($(($samples/1000+10))*$outer*4/60+10)) minitues. Reduce 'outer' if you want a shorter run."
# Prepare
# sudo rm -rf out
# mkdir out
# sudo rm -rf input.txt


for guess_k in {1..3328}
# for ((guess_k=7601; guess_k<=7601+49; guess_k++))
do
    sudo ./test_kyber_indcpa_local $guess_k 0  1 1 
done
# Warm Up
# stress-ng -q --cpu $TOTAL_LOGICAL_CORES -t 10m
# Runn
# sudo ./bin/driver_512-384_fix ${samples} ${outer}
# cp -r out/ data/out-${date}




# done






# Unload MSR module
# sudo modprobe -r msr