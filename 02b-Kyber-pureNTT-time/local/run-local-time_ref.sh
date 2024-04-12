#!/usr/bin/env bash
TOTAL_PHYSICAL_CORES=`grep '^core id' /proc/cpuinfo | sort -u | wc -l`
TOTAL_LOGICAL_CORES=`grep '^core id' /proc/cpuinfo | wc -l`
# stress-ng -q --cpu $TOTAL_LOGICAL_CORES -t 10m
thread=8
iteration=1
outer=10000
for ((i=1;i<=$outer;i++))
do
# for guess_z in {1..3328}
# RIGHT S 1760/2923
for guess_z in 1760 2011
do
../kyber/ref/test_kyber_ntt_local $guess_z 0 $thread $iteration
done
done


