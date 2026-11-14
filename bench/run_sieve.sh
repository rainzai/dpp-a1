#!/bin/bash
# run_sieve.sh: times the sieve on a DAS-5 node for 100, 1000 and 5000 primes
# with 1-16 cores (taskset), 3 runs each, and appends the results to a CSV.
# Usage on fs2: module load prun; nohup bash run_sieve.sh > sieve.log 2>&1 &

D=$HOME/sean/dpp-a1
OUT=$HOME/sean/results/sieve.csv

mkdir -p "$(dirname "$OUT")"
[ -f "$OUT" ] || echo "primes,cores,run,seconds" > "$OUT"

for r in 1 2 3; do
    for n in 100 1000 5000; do
        for c in 1 2 4 8 16; do
            secs=$(prun -np 1 /bin/bash "$D/bench/time_sieve.sh" "$c" "$n" "$D/1.3/assign1_3")
            echo "$n,$c,$r,$secs" >> "$OUT"
            echo "primes=$n cores=$c run=$r: $secs s"
        done
    done
done
echo "DONE"
