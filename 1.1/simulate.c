#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "simulate.h"

// shared by all threads
double *old, *cur, *next;
int g_i_max, g_t_max;
pthread_barrier_t barrier;

// data for each thread
typedef struct {
    int start, end;
    int id;
} t_data;

// computes its own range of points for every time step.
void* thread(void* vargp) {
    t_data* d = (t_data*)vargp;
    const double c = 0.15;

    for (int t = 0; t < g_t_max; t++) {
        for (int i = d->start; i < d->end; i++) {
            // given formula
            next[i] = 2 * cur[i] - old[i] +
                      c * (cur[i - 1] - (2 * cur[i] - cur[i + 1]));
        }
        pthread_barrier_wait(&barrier);

        //  rotates the shared buffers
        if (d->id == 0) {
            double* tmp = old;
            old = cur;
            cur = next;
            next = tmp;
        }
        // wait until the swap is done
        pthread_barrier_wait(&barrier);
    }
    return NULL;
}

/*
 * Executes the entire simulation.
 *
 * i_max: how many data points are on a single wave
 * t_max: how many iterations the simulation should run
 * num_threads: how many threads to use (excluding the main threads)
 * old_array: array of size i_max filled with data for t-1
 * current_array: array of size i_max filled with data for t
 * next_array: array of size i_max. You should fill this with t+1
 */
double* simulate(const int i_max, const int t_max, const int num_threads,
                 double* old_array, double* current_array, double* next_array) {
    old = old_array;
    cur = current_array;
    next = next_array;
    g_i_max = i_max;
    g_t_max = t_max;

    pthread_t* thd = (pthread_t*)malloc(sizeof(pthread_t) * num_threads);
    t_data* myData = (t_data*)malloc(sizeof(t_data) * num_threads);

    pthread_barrier_init(&barrier, NULL, num_threads);

    int length = i_max - 2;
    for (int i = 0; i < num_threads; i++) {
        myData[i].id = i;
        myData[i].start = 1 + i * length / num_threads;
        myData[i].end = 1 + (i + 1) * length / num_threads;
        pthread_create(&thd[i], NULL, thread, (void*)&myData[i]);
    }
    for (int i = 0; i < num_threads; i++)
        pthread_join(thd[i], NULL);

    pthread_barrier_destroy(&barrier);
    free(thd);
    free(myData);

    /* You should return a pointer to the array with the final results. */
    return cur;
}
