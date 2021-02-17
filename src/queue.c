#include "queue.h"

#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * In this queue implementation the node pointed to by head is the sentinel
 * node. Upon dequeuing a node, the second node becomes the new sentinel
 * node. The must always be at least one (sentinel) node in the queue, i.e.,
 * head and tail are never NULL.
 * FIXME: this queue is not protected against the ABA problem.
 */

void *queue_deq(struct queue *queue)
{
	struct node *first, *last, *next;
	while (1) {
		first = atomic_load_explicit(&queue->head, memory_order_relaxed);
		last = atomic_load_explicit(&queue->tail, memory_order_relaxed);
		next = atomic_load_explicit(&first->next, memory_order_relaxed);
		if (first == atomic_load_explicit(&queue->head, memory_order_acquire)) {
			if (first == last) {
				if (next == NULL) {
					return NULL;
				}
				atomic_compare_exchange_strong_explicit(&queue->tail, &last, next, memory_order_acq_rel, memory_order_relaxed);
			} else {
				if (atomic_compare_exchange_strong_explicit(&queue->head, &first, next, memory_order_acq_rel, memory_order_relaxed)) {
					free(first);
					return next->data;
				}
			}
		}
	}
}

int queue_enq(struct queue *queue, void *x)
{
	struct node *last, *next;
	struct node *n = calloc(1, sizeof(*n));
	if (n == NULL) {
		perror("calloc");
		return 1;
	}
	n->data = x;
	while (1) {
		last = atomic_load_explicit(&queue->tail, memory_order_relaxed);
		next = atomic_load_explicit(&last->next, memory_order_relaxed);
		if (last == atomic_load_explicit(&queue->tail, memory_order_acquire)) {
			if (next == NULL) {
				if (atomic_compare_exchange_strong_explicit(&last->next, &next, n, memory_order_acq_rel, memory_order_relaxed)) {
					atomic_compare_exchange_strong_explicit(&queue->tail, &last, n, memory_order_acq_rel, memory_order_relaxed);
					return 0;
				}
			} else {
				atomic_compare_exchange_strong_explicit(&queue->tail, &last, next, memory_order_acq_rel, memory_order_relaxed);
			}
		}
	}
}

int queue_init(struct queue *queue)
{
	struct node *n = calloc(1, sizeof(*n));
	if (n == NULL) {
		perror("calloc");
		return 1;
	}
	atomic_store_explicit(&queue->head, n, memory_order_release);
	atomic_store_explicit(&queue->tail, queue->head, memory_order_release);

	return 0;
}
