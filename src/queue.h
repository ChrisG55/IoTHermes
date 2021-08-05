#ifndef QUEUE_H
#define QUEUE_H

#include <stdatomic.h>

struct node {
	void *data;
	_Atomic(struct node *)next;
	unsigned char type;
};

struct queue {
	_Atomic(struct node *)head;
	_Atomic(struct node *)tail;
};

void *queue_deq(struct queue *queue, unsigned char *type);
int queue_enq(struct queue *queue, void *x, unsigned char type);
int queue_init(struct queue *queue);

#endif /* QUEUE_H */
