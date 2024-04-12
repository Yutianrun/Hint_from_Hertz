#!/bin/bash
thread=300
iteration=10000

for ((outer=1;outer<=5;outer++))
do
for guess_z in {1..3328}
do
# ../kyber/ref/test_kyber_indcpa_local $guess_z 0 $thread $iteration
../kyber/avx2/test_kyber_indcpa_local $guess_z 0 $TOTAL_LOGICAL_CORES $iteration
done
done
