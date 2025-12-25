#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "common.h"

#define QUEUE_SIZE MAX_DEFAULT

typedef Idx QueueIdx;

struct Queue {
	Idx items[QUEUE_SIZE];
	QueueIdx front;
	QueueIdx back;
};

struct QueueIterator {
	struct Queue *queue;
	QueueIdx current;
};

void queueInit(struct Queue *queue);
bool queueIsEmpty(struct Queue *queue);
bool queueIsFull(struct Queue *queue);
QueueIdx queueSize(struct Queue *queue);
void queuePush(struct Queue *queue, Idx item);
Idx queuePop(struct Queue *queue);
Idx queuePeek(struct Queue *queue);
struct QueueIterator queueIterate(struct Queue *queue);
bool queueIteratorNext(struct QueueIterator *iter, Idx *out);
