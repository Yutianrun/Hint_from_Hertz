#!/bin/bash

# Run all selectors directly
stress-ng -q --cpu 6 -t 10m
thread=10000
iteration=300

../NTTRU/ref/test/test_kem_local 0 0 $thread $iteration
../NTTRU/ref/test/test_kem_local 1 0 $thread $iteration 
../NTTRU/ref/test/test_kem_local 0 5 $thread $iteration
../NTTRU/ref/test/test_kem_local 1 5 $thread $iteration
../NTTRU/ref/test/test_kem_local 0 14 $thread $iteration
../NTTRU/ref/test/test_kem_local 1 14 $thread $iteration
../NTTRU/ref/test/test_kem_local 0 19 $thread $iteration
../NTTRU/ref/test/test_kem_local 1 19 $thread $iteration
../NTTRU/ref/test/test_kem_local 0 33 $thread $iteration
../NTTRU/ref/test/test_kem_local 1 33 $thread $iteration
../NTTRU/ref/test/test_kem_local 0 60 $thread $iteration
../NTTRU/ref/test/test_kem_local 1 60 $thread $iteration
../NTTRU/ref/test/test_kem_local 0 94 $thread $iteration
../NTTRU/ref/test/test_kem_local 1 94 $thread $iteration
../NTTRU/ref/test/test_kem_local 0 97 $thread $iteration
../NTTRU/ref/test/test_kem_local 1 97 $thread $iteration
../NTTRU/ref/test/test_kem_local 0 106 $thread $iteration
../NTTRU/ref/test/test_kem_local 1 106 $thread $iteration
../NTTRU/ref/test/test_kem_local 0 117 $thread $iteration
../NTTRU/ref/test/test_kem_local 1 117 $thread $iteration
../NTTRU/ref/test/test_kem_local 0 124 $thread $iteration
../NTTRU/ref/test/test_kem_local 1 124 $thread $iteration
../NTTRU/ref/test/test_kem_local 0 131 $thread $iteration
../NTTRU/ref/test/test_kem_local 1 131 $thread $iteration
../NTTRU/ref/test/test_kem_local 0 136 $thread $iteration
../NTTRU/ref/test/test_kem_local 1 136 $thread $iteration
../NTTRU/ref/test/test_kem_local 0 163 $thread $iteration
../NTTRU/ref/test/test_kem_local 1 163 $thread $iteration
../NTTRU/ref/test/test_kem_local 0 173 $thread $iteration
../NTTRU/ref/test/test_kem_local 1 173 $thread $iteration
../NTTRU/ref/test/test_kem_local 0 208 $thread $iteration
../NTTRU/ref/test/test_kem_local 1 208 $thread $iteration
../NTTRU/ref/test/test_kem_local 0 220 $thread $iteration
../NTTRU/ref/test/test_kem_local 1 220 $thread $iteration
../NTTRU/ref/test/test_kem_local 0 221 $thread $iteration
../NTTRU/ref/test/test_kem_local 1 221 $thread $iteration
../NTTRU/ref/test/test_kem_local 0 247 $thread $iteration
../NTTRU/ref/test/test_kem_local 1 247 $thread $iteration
../NTTRU/ref/test/test_kem_local 0 248 $thread $iteration
../NTTRU/ref/test/test_kem_local 1 248 $thread $iteration
