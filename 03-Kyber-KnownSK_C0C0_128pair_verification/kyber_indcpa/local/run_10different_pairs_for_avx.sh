#!/usr/bin/env bash

TOTAL_PHYSICAL_CORES=`grep '^core id' /proc/cpuinfo | sort -u | wc -l`
TOTAL_LOGICAL_CORES=`grep '^core id' /proc/cpuinfo | wc -l`

# Load MSR module
sudo modprobe msr

# Setup
samples=100000	# 100 seconds
outer=5
date=`date +"%m%d-%H%M"`
thread=8 #FIXME
range_to_search=2

# Alert
echo "This script will take about $(($(($samples/1000+30))*$outer*10/60+10)) minitues. Reduce 'outer' if you want a shorter run."

# Prepare
sudo rm -rf out
mkdir out
sudo rm -rf input.txt

# fixed the pair of index 0
# Ten differnt pairs of indexes.

# RIGHT z 1228/3273 for (sk.coeff[10] ,sk.coeff[11])
for guess_z in  1228  3273
do
echo $guess_z 5 $thread >> input.txt
done

# RIGHT z 809/3016 for (sk.coeff[88] ,sk.coeff[89])
for guess_z in  809 3016
do
echo $guess_z 44 $thread >> input.txt
done


# RIGHT z 1119/2528 for (sk.coeff[106] ,sk.coeff[107])
for guess_z in 1119 2528
do
echo $guess_z 53 $thread >> input.txt
done

# RIGHT z 1579/2012 for (sk.coeff[112] ,sk.coeff[113])
for guess_z in 1579 2012
do
echo $guess_z 56 $thread >> input.txt
done


# RIGHT z 253/2658 for (sk.coeff[120] ,sk.coeff[121])
for guess_z in  253 2658
do
echo $guess_z 60 $thread >> input.txt
done


# RIGHT z 1224/2758 for (sk.coeff[132] ,sk.coeff[133])
for guess_z in  1224 2758
do
echo $guess_z 66 $thread >> input.txt
done


# RIGHT z 1313/2750 for (sk.coeff[138] ,sk.coeff[139])
for guess_z in 1313 2750
do
echo $guess_z 69 $thread >> input.txt
done


# RIGHT z 567/1604 for (sk.coeff[148] ,sk.coeff[149])
for guess_z in 567 1604 
do
echo $guess_z 74 $thread >> input.txt
done


# RIGHT z 21/1869 for (sk.coeff[194] ,sk.coeff[195])
for guess_z in 21 1869
do
echo $guess_z 97 $thread >> input.txt
done

# RIGHT z 1803/2443 for (sk.coeff[212] ,sk.coeff[213])
for guess_z in 1803 2443
do
echo $guess_z 106 $thread >> input.txt
done

# Warm Up
stress-ng -q --cpu $TOTAL_LOGICAL_CORES -t 10m

# Run
sudo ./bin/driver_indcpa_avx_two_pair ${samples} ${outer}
cp -r out data/out-${date}-two-pair
# Unload MSR module
sudo modprobe -r msr
