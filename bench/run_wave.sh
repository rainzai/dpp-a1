#!/bin/bash
# run_wave.sh: runs all wave-equation benchmarks on DAS-5 via prun and
# appends one CSV line per run.
# Usage on fs2: module load prun; nohup bash run_wave.sh > run.log 2>&1 &

B=$HOME/sean/dpp-a1
OUT=$HOME/sean/results/wave.csv
REPS=3
THREADS="1 2 4 6 8 12 14 16"
# i_max:t_max, chosen so the sequential run takes about 2-40 s
SIZES="1000:1000000 10000:1000000 100000:200000 1000000:20000 10000000:2000"

mkdir -p "$(dirname "$OUT")"
cd "$(dirname "$OUT")"   # each run's result.txt lands here, not in the code
[ -f "$OUT" ] || echo "program,i_max,t_max,threads,schedule,chunk,run,seconds" > "$OUT"

# measure NAME SCHEDULE CHUNK I_MAX T_MAX THREADS RUN PROGRAM
measure() {
    name=$1; sched=$2; chunk=$3; n=$4; t=$5; p=$6; r=$7; prog=$8
    if [ "$chunk" = "-" ]; then omp=$sched; else omp=$sched,$chunk; fi
    secs=$(prun -np 1 /usr/bin/env OMP_SCHEDULE="$omp" "$prog" "$n" "$t" "$p" \
           | awk '/Took/ {print $2}')
    echo "$name,$n,$t,$p,$sched,$chunk,$r,$secs" >> "$OUT"
    echo "$name n=$n t=$t p=$p $omp run=$r: $secs s"
}

# Main experiment: all sizes and thread counts
for size in $SIZES; do
    n=${size%:*}; t=${size#*:}
    for r in $(seq 1 $REPS); do
        measure seq - - "$n" "$t" 1 "$r" "$B/seq/assign1_1"
        for p in $THREADS; do
            measure pthreads - - "$n" "$t" "$p" "$r" "$B/1.1/assign1_1"
            measure openmp static - "$n" "$t" "$p" "$r" "$B/1.2/assign1_2"
        done
    done
done

# Schedule experiment: OpenMP at 10^6 points
for r in $(seq 1 $REPS); do
    for p in 4 8 16; do
        for s in static:1000 dynamic:1000 dynamic:10000 guided:-; do
            measure openmp "${s%:*}" "${s#*:}" 1000000 20000 "$p" "$r" "$B/1.2/assign1_2"
        done
    done
done
echo "DONE"
