#!/usr/bin/env bash
TOTAL_PHYSICAL_CORES=`grep '^core id' /proc/cpuinfo | sort -u | wc -l`
TOTAL_LOGICAL_CORES=`grep '^core id' /proc/cpuinfo | wc -l`
thread=300
iteration=10000
outer=1
for ((i=1;i<=$outer;i++))
do
# for guess_z in {1..3328}
# RIGHT S 1760/2923
for guess_z in 1760 2011
do
../kyber/avx2/test_kyber_ntt_local $guess_z 0 $thread $iteration
done
done
