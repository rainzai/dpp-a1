/*
 * simulate.c
 *
 * Implement your (parallel) simulation here!
 */

#include <stdio.h>
#include <stdlib.h>

#include "simulate.h"


/*
 * Executes the entire simulation.
 *
 * Implement your code here.
 *
 * i_max: how many data points are on a single wave
 * t_max: how many iterations the simulation should run
 * num_threads: how many threads to use (the main thread is one of them)
 * old_array: array of size i_max filled with data for t-1
 * current_array: array of size i_max filled with data for t
 * next_array: array of size i_max. You should fill this with t+1
 */
double* simulate(const int i_max, const int t_max, const int num_threads,
                 double* old_array, double* current_array, double* next_array) {

    const double c = 0.15;

    for (int t = 0; t < t_max; t++) {
        /*
         * Split the i loop over the threads: within one time step each point
         * only reads current_array and old_array, so all points are
         * independent. The t loop stays sequential, because step t+1 needs
         * the whole of step t. schedule(runtime) reads the scheduling policy
         * from the OMP_SCHEDULE environment variable.
         */
        #pragma omp parallel for num_threads(num_threads) schedule(runtime)
        for (int i = 1; i < i_max - 1; i++) {
            // given formula
            next_array[i] = 2 * current_array[i] - old_array[i] +
                            c * (current_array[i - 1] -
                                 (2 * current_array[i] - current_array[i + 1]));
        }

        /*
         * Safe without a lock: the parallel for ends with a barrier, so all
         * threads have finished this time step and only the main thread runs
         * the swap.
         */
        double* tmp = old_array;
        old_array = current_array;
        current_array = next_array;
        next_array = tmp;
    }

    /* You should return a pointer to the array with the final results. */
    return current_array;
}
