/*
 * assign1_3.c
 *
 * Multithreaded Sieve of Eratosthenes. Generator thread sends 2, 3, 4, ...
 * into pipeline of filter threads that are connected by bounded queues.
 * Filter k prints the k-th prime and removes its multiples from the stream.
 * The program exits after printing the requested number of primes.
 *
 * Usage: ./assign1_3 <number of primes>
 *        (the last argument is used, so "./assign1 3 <n>" also works)
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

/* Small thread stacks, so thousands of filter threads fit in memory. */
#define STACK_SIZE (64 * 1024)

static int num_primes; /* number of primes to print before exiting */

/* Arguments for 1 filter thread. */
typedef struct {
    queue_t *in; /* queue with the prime candidates for this filter */
    int index;   /* this filter prints the index-th prime (counted from 1) */
} filter_arg_t;

static void start_thread(void *(*func)(void *), void *arg);

/* Generator: sends 2, 3, 4, ... to its output queue, forever. */
static void *generator(void *arg) {
    queue_t *out = arg;

    for (int n = 2;; n++) {
        queue_put(out, n);
    }
    return NULL;
}

/*
 * Filter: first number received is a prime, which is printed. All later
 * numbers that aren't a multiple of it are forwarded to the next filter,
 * which is created when the first such number arrives.
 */
static void *filter(void *arg) {

    filter_arg_t *f = arg;
    queue_t *out = NULL;

    int prime = queue_take(f->in);
    printf("%d\n", prime);

    if (f->index == num_primes) {
        fflush(stdout);
        exit(EXIT_SUCCESS); /* stops all threads */
    }

    for (;;) {
        int n = queue_take(f->in);
        if (n % prime == 0) {
            continue;
        }

        if (out == NULL) {
            filter_arg_t *next = malloc(sizeof(filter_arg_t));

            if (next == NULL) {
                fprintf(stderr, "Could not allocate filter arguments.\n");
                exit(EXIT_FAILURE);
            }

            out = queue_create();
            next->in = out;
            next->index = f->index + 1;
            start_thread(filter, next);
        }
        queue_put(out, n);
    }
    return NULL;
}

/* Starts a detached thread with a small stack, exits on failure. */
static void start_thread(void *(*func)(void *), void *arg) {
    pthread_attr_t attr;
    pthread_t tid;

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, STACK_SIZE);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    if (pthread_create(&tid, &attr, func, arg) != 0) {
        fprintf(stderr, "Could not create thread.\n");
        exit(EXIT_FAILURE);
    }

    pthread_attr_destroy(&attr);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <number of primes>\n", argv[0]);
        return EXIT_FAILURE;
    }

    num_primes = atoi(argv[argc - 1]);

    if (num_primes < 1) {
        printf("argument error: number of primes should be >= 1.\n");
        return EXIT_FAILURE;
    }

    filter_arg_t *first = malloc(sizeof(filter_arg_t));
    if (first == NULL) {
        fprintf(stderr, "Could not allocate filter arguments.\n");
        return EXIT_FAILURE;
    }

    first->in = queue_create();
    first->index = 1;

    start_thread(filter, first);
    start_thread(generator, first->in);

    /* main has nothing left to do. The last filter ends the program. */
    pthread_exit(NULL);
}
