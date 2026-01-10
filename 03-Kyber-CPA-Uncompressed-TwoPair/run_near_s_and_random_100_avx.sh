#!/usr/bin/env bash
# Example runner for the two-pair setting:
#   - pair0 is fixed to (1, z_fixed)
#   - target pair "pair_index" is set to (1, z_target) and z_target is varied

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

# Fixed z at pair0 (choose one of the two values from:
#   ./bin/check_03_consistency --scan 0
# e.g. 2246 or 3165 for the default deterministic key in this repo)
z_fixed=3165

# Which pair to attack (k). Must be != 0 to actually get a two-pair ciphertext.
pair_index=5

# Candidate z_target values.
# Tip: use the offline scanner to find the 2 "half-zero" z's for this pair:
#   ./bin/check_03_consistency --scan-two-pair ${z_fixed} ${pair_index}
for guess_z in 1 30 75 100 185 251 374 388 465 494 516 530 531 565 626 638 651 725 756 811 845 888 903 918 934 941 987 1018 1061 1082 1141 1207 1220 1221 1251 1279 1291 1297 1300 1301 1327 1339 1345 1357 1399 1403 1423 1475 1522 1631 1642 1646 1676 1694 1712 1723 1727 1737 1778 1784 1793 1802 1805 1814 1820 1885 1893 1915 1986 2008 2013 2034 2099 2202 2240 2248 2264 2294 2303 2374 2440 2454 2480 2498 2549 2604 2612 2661 2795 2886 2901 2904 2928 2968 2997 3010 3077 3174 3268
do
echo ${z_fixed} $guess_z ${pair_index} $thread >> input.txt
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
