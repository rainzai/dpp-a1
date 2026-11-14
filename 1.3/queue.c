/*
 * queue.c
 *
 * Bounded queue protected by one mutex and two condition variables.
 */

#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

/* Allocates and initialises an empty queue. Exits if out of memory. */
queue_t *queue_create(void)
{
    queue_t *q = malloc(sizeof(queue_t));
    if (q == NULL) {
        fprintf(stderr, "Could not allocate queue.\n");
        exit(EXIT_FAILURE);
    }
    q->head = 0;
    q->count = 0;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->not_full, NULL);
    pthread_cond_init(&q->not_empty, NULL);
    return q;
}

/* Appends value; blocks while the queue is full. */
void queue_put(queue_t *q, int value)
{
    pthread_mutex_lock(&q->lock);
    while (q->count == QUEUE_SIZE) {
        pthread_cond_wait(&q->not_full, &q->lock);
    }
    q->buf[(q->head + q->count) % QUEUE_SIZE] = value;
    q->count++;
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->lock);
}

/* Removes and returns the oldest value; blocks while the queue is empty. */
int queue_take(queue_t *q)
{
    pthread_mutex_lock(&q->lock);
    while (q->count == 0) {
        pthread_cond_wait(&q->not_empty, &q->lock);
    }
    int value = q->buf[q->head];
    q->head = (q->head + 1) % QUEUE_SIZE;
    q->count--;
    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->lock);
    return value;
}
