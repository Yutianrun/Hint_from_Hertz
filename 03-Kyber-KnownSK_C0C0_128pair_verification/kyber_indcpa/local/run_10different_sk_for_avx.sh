#!/usr/bin/env bash

TOTAL_PHYSICAL_CORES=`grep '^core id' /proc/cpuinfo | sort -u | wc -l`
TOTAL_LOGICAL_CORES=`grep '^core id' /proc/cpuinfo | wc -l`

# Load MSR module
sudo modprobe msr

# Setup
samples=20000	# 100 seconds
outer=5
date=`date +"%m%d-%H%M"`
thread=8 #FIXME
range_to_search=2

# Alert
echo "This script will take about $(($(($samples/1000+20))*$outer*4/60+10)) minitues. Reduce 'outer' if you want a shorter run."

# Prepare
sudo rm -rf out
mkdir out
sudo rm -rf input.txt

# Only target index 0 



# RIGHT z 1662/2859 for sk[0]
for guess_z in 1035 1662 2859 2385
do
echo $guess_z 0 $thread 0 >> input.txt
done

# RIGHT z 1178/3265 for sk[1]
for guess_z in 1178 1596 2614 3265
do
echo $guess_z 0 $thread 1 >> input.txt
done


# RIGHT z 442/3294 for sk[2]
for guess_z in 442 1561 3174 3294
do
echo $guess_z 0 $thread 2 >> input.txt
done


# RIGHT z 959/2268 for sk[3]
for guess_z in 959 2268 2699 3174
do
echo $guess_z 0 $thread 3 >> input.txt
done

# RIGHT z 122/1947 for sk[4]
for guess_z in 122 1947 2087 2807
do
echo $guess_z 0 $thread 4 >> input.txt
done


# RIGHT z 864/1296 for sk[5]
for guess_z in  789 864 1196 1296 
do
echo $guess_z 0 $thread 5 >> input.txt
done


# RIGHT z 399/2823 for sk[6]
for guess_z in  284 399 1401 2823
do
echo $guess_z 0 $thread 6 >> input.txt
done


# RIGHT z 225/745 for sk[7]
for guess_z in 225 470 745 908
do
echo $guess_z 0 $thread 7 >> input.txt
done


# RIGHT z 1190/3316 for sk[8]
for guess_z in 726 1190 1528 3316 
do
echo $guess_z 0 $thread 8 >> input.txt
done


# RIGHT z 927/1409 for sk[9]
for guess_z in 927 1409 2264 2671
do
echo $guess_z 0 $thread 9 >> input.txt
done

# Warm Up
stress-ng -q --cpu $TOTAL_LOGICAL_CORES -t 10m

# Run
sudo ./bin/driver_indcpa_avx ${samples} ${outer}
cp -r out data/out-${date}
# Unload MSR module
sudo modprobe -r msr
