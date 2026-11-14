#!/bin/bash
# time_sieve.sh CORES PRIMES PROGRAM: runs PROGRAM PRIMES on the first CORES
# cores of this machine and prints the elapsed wall-clock time in seconds.
start=$(date +%s.%N)
taskset -c 0-$(($1 - 1)) "$3" "$2" > /dev/null
end=$(date +%s.%N)
awk -v s="$start" -v e="$end" 'BEGIN { printf "%.4f\n", e - s }'
