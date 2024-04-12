#!/usr/bin/env bash

TOTAL_PHYSICAL_CORES=`grep '^core id' /proc/cpuinfo | sort -u | wc -l`
TOTAL_LOGICAL_CORES=`grep '^core id' /proc/cpuinfo | wc -l`

# Load MSR module
sudo modprobe msr

# Setup
samples=20000	# 200 seconds
outer=10
date=`date +"%m%d-%H%M"`
thread=8

echo "This script will take about $(($(($samples/1000+10))*$outer*100/60+10)) minitues. Reduce 'outer' if you want a shorter run."
# Prepare
sudo rm -rf out
mkdir out
sudo rm -rf input.txt



#942 6684 7359 correct z
# for guess_z in   922 942   1646 1853 2326  6684  7000  7359   7400 7548
# do
# echo $guess_z 0  $thread >> input.txt
# done
for guess_z in   127 341 418 422 424 472 537 579 618 663 710 798 816 834 887 942 1015 1199 1344 1378 1392 1444 1494 1565 1582 1586 1707 1721 1825 1833 2193 2258 2431 2684 2895 3123 3214 3352 3354 3414 3463 3573 3795 3894 4039 4046 4084 4095 4122 4209 4219 4252 4355 4357 4379 4445 4466 4503 4847 4863 4898 4915 5012 5157 5186 5219 5314 5353 5370 5382 5384 5553 5715 5728 5855 5870 6041 6130 6145 6326 6365 6534 6573 6603 6684 6704 6836 6837 6908 6919 7082 7152 7359 7372 7417 7462 7500 7607 7626 7639
do
echo $guess_z 0  $thread >> input.txt
done
#               random z

# Write selectors
# echo 942  0 $thread>> input.txt
# echo 1370 0 $thread>> input.txt
# echo 1499 0 $thread>> input.txt
# echo 3126 0 $thread>> input.txt
# echo 3266 0 $thread>> input.txt
# echo 3535 0 $thread>> input.txt
# echo 0 19 $thread>> input.txt
# echo 1 19 $thread>> input.txt
# echo 0 33 $thread>> input.txt
# echo 1 33 $thread>> input.txt
# echo 0 60 $thread>> input.txt
# echo 1 60 $thread>> input.txt
# echo 0 94 $thread>> input.txt
# echo 1 94 $thread>> input.txt
# echo 0 97 $thread>> input.txt
# echo 1 97 $thread>> input.txt
# echo 0 106 $thread>> input.txt
# echo 1 106 $thread>> input.txt
# echo 0 117 $thread>> input.txt
# echo 1 117 $thread>> input.txt
# echo 0 124 $thread>> input.txt
# echo 1 124 $thread>> input.txt
# echo 0 131 $thread>> input.txt
# echo 1 131 $thread>> input.txt
# echo 0 136 $thread>> input.txt
# echo 1 136 $thread>> input.txt
# echo 0 163 $thread>> input.txt
# echo 1 163 $thread>> input.txt
# echo 0 173 $thread>> input.txt
# echo 1 173 $thread>> input.txt
# echo 0 208 $thread>> input.txt
# echo 1 208 $thread>> input.txt
# echo 0 220 $thread>> input.txt
# echo 1 220 $thread>> input.txt
# echo 0 221 $thread>> input.txt
# echo 1 221 $thread>> input.txt
# echo 0 247 $thread>> input.txt
# echo 1 247 $thread>> input.txt
# echo 0 248 $thread>> input.txt
# echo 1 248 $thread>> input.txt

# Warm Up
stress-ng -q --cpu $TOTAL_LOGICAL_CORES -t 10m

# Run
sudo ./bin/driver_cpa ${samples} ${outer}
cp -r out/ data/out-${date}

# Unload MSR module
sudo modprobe -r msr