/*
 * queue.h
 * Bounded, thread-safe FIFO queue of integers. Producers block while the
 * queue is full, consumers block while it is empty.
 */

#pragma once
#include <pthread.h>
#define QUEUE_SIZE 16

typedef struct {
    int buf[QUEUE_SIZE];       /* ring buffer with the stored numbers */
    int head;                  /* index of the oldest element */
    int count;                 /* number of elements currently stored */
    pthread_mutex_t lock;      /* protects buf, head and count */
    pthread_cond_t not_full;   /* signalled after a take */
    pthread_cond_t not_empty;  /* signalled after a put */
} queue_t;

queue_t *queue_create(void);
void queue_put(queue_t *q, int value);
int queue_take(queue_t *q);
