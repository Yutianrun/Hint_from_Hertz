#! /bin/bash
TOTAL_PHYSICAL_CORES=`grep '^core id' /proc/cpuinfo | sort -u | wc -l`
TOTAL_LOGICAL_CORES=`grep '^core id' /proc/cpuinfo | wc -l`

# Load MSR module
sudo modprobe msr

# for i in {1,101,201,351,601,701,751,851,1001,1051,1151,1551,1651,2001,2051,2251,2451,2501,2701,3001}

# do

# Setup
samples=20000   # 20 seconds
outer=10
date=`date +"%m%d-%H%M"`
thread=8

echo "This script will take about $(($(($samples/1000+10))*$outer*100/60+10)) minitues. Reduce 'outer' if you want a shorter run."
# Prepare
sudo rm -rf out
mkdir out
sudo rm -rf input.txt

# right z 110 597
for  guess_z in 2 45 64 110 144 229 230 243 284 295 306 334 383 422 509 544 567 597 691 716 749 803 849 862 952 1031 1088 1158 1162 1211 1224 1330 1337 1354 1489 1589 1594 1606 1621 1633 1654 1670 1683 1689 1737 1739 1762 1764 1772 1808 1820 1845 1874 1876 1893 1900 1911 1977 2014 2054 2056 2060 2062 2083 2155 2157 2307 2327 2348 2370 2371 2396 2410 2541 2573 2607 2642 2704 2721 2724 2785 2808 2817 2874 2895 2959 2970 3005 3015 3035 3128 3180 3228 3229 3234 3247 3257 3258 3312 3322

# for ((guess_k=7601; guess_k<=7601+49; guess_k++))
do
echo  $guess_z 0 $thread >> input.txt
done
# Warm Up
stress-ng -q --cpu $TOTAL_LOGICAL_CORES -t 10m
# Run
sudo ./bin/driver ${samples} ${outer} 
cp -r out/ data/out-${date}

# sleep 1800

# done

# Unload MSR module
sudo modprobe -r msr
