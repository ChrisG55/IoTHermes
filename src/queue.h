#ifndef QUEUE_H
#define QUEUE_H

#include <stdatomic.h>

struct node {
	void *data;
	_Atomic(struct node *)next;
};

struct queue {
	_Atomic(struct node *)head;
	_Atomic(struct node *)tail;
};

void *queue_deq(struct queue *queue);
int queue_enq(struct queue *queue, void *x);
int queue_init(struct queue *queue);

#endif /* QUEUE_H */
