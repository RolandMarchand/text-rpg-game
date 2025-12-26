#include <assert.h>
#include <limits.h>
#include <string.h>

#include "queue.h"

_Static_assert(1ull << sizeof(QueueIdx) * CHAR_BIT >= QUEUE_SIZE,
	"QueueIdx's type cannot address all values of QUEUE_SIZE");

void queueInit(struct Queue *queue)
{
	assert(queue != NULL);

	memset(queue, 0, sizeof(struct Queue));
}

bool queueIsEmpty(struct Queue *queue)
{
	assert(queue != NULL);

	return queue->front == queue->back;
}

bool queueIsFull(struct Queue *queue)
{
	assert(queue != NULL);

	return ((queue->back + 1) % QUEUE_SIZE) == queue->front;
}

QueueIdx queueSize(struct Queue *queue)
{
	assert(queue != NULL);

	return (queue->back - queue->front + QUEUE_SIZE) % QUEUE_SIZE;
}

bool queuePush(struct Queue *queue, Idx item)
{
	assert(queue != NULL);

	if (queueIsFull(queue)) {
		return false;
	}

	queue->items[queue->back] = item;
	queue->back = (queue->back + 1) % QUEUE_SIZE;

	return true;
}

Idx queuePop(struct Queue *queue)
{
	assert(queue != NULL);

	if (queueIsEmpty(queue)) {
		return -1;
	}

	Idx item = queue->items[queue->front];
	queue->front = (queue->front + 1) % QUEUE_SIZE;
	return item;
}

Idx queuePeek(struct Queue *queue)
{
	assert(queue != NULL);

	if (queueIsEmpty(queue)) {
		return -1;
	}

	return queue->items[queue->front];
}

struct QueueIterator queueIterate(struct Queue *queue)
{
	assert(queue != NULL);

	struct QueueIterator iter = { 0 };
	iter.queue = queue;
	iter.current = queue->front;

	return iter;
}

bool queueIteratorNext(struct QueueIterator *iter, Idx *out)
{
	assert(iter != NULL);
	assert(iter->queue != NULL);
	assert(out != NULL);

	if (iter->current == iter->queue->back) {
		return false;
	}

	*out = iter->queue->items[iter->current];
	iter->current = (iter->current + 1) % QUEUE_SIZE;

	return true;
}
