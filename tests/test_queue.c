#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "queue.h"
#include "unity/unity.h"

#define ALLOC(x) (obstack_alloc(&arena, x))

struct obstack arena;

void setUp(void)
{
	obstack_init(&arena);
}

void tearDown(void)
{
	obstack_free(&arena, NULL);
}

struct Queue *newQueue(void)
{
	struct Queue *q = ALLOC(sizeof(struct Queue));
	TEST_ASSERT_NOT_NULL(q);
	queueInit(q);
	return q;
}

void testInitInternal(struct Queue *q)
{
	queueInit(q);

	TEST_ASSERT_EQUAL_UINT16(0, q->front);
	TEST_ASSERT_EQUAL_UINT16(0, q->back);
	TEST_ASSERT_TRUE(queueIsEmpty(q));
	TEST_ASSERT_FALSE(queueIsFull(q));
	TEST_ASSERT_EQUAL_UINT16(0, queueSize(q));
}

void testInit(void)
{
	struct Queue *q = ALLOC(sizeof(struct Queue));
	TEST_ASSERT_NOT_NULL(q);
	memset(q, 0, sizeof(struct Queue));
	testInitInternal(q);
}

void testInitDirty(void)
{
	struct Queue *q = ALLOC(sizeof(struct Queue));
	TEST_ASSERT_NOT_NULL(q);
	memset(q, 0xBB, sizeof(struct Queue));
	testInitInternal(q);
}

void testPushPop(void)
{
	struct Queue *q = newQueue();

	TEST_ASSERT_TRUE(queuePush(q, 42));
	TEST_ASSERT_FALSE(queueIsEmpty(q));
	TEST_ASSERT_EQUAL_UINT16(1, queueSize(q));
	TEST_ASSERT_EQUAL_UINT16(42, queuePeek(q));

	Idx item = queuePop(q);
	TEST_ASSERT_EQUAL_UINT16(42, item);
	TEST_ASSERT_TRUE(queueIsEmpty(q));
	TEST_ASSERT_EQUAL_UINT16(0, queueSize(q));
}

void testPushMultiple(void)
{
	struct Queue *q = newQueue();

	for (Idx i = 1; i <= 10; i++) {
		TEST_ASSERT_TRUE(queuePush(q, i));
		TEST_ASSERT_EQUAL_UINT16(i, queueSize(q));
		TEST_ASSERT_EQUAL_UINT16(1, queuePeek(q));
		TEST_ASSERT_FALSE(queueIsEmpty(q));
	}

	for (Idx i = 1; i <= 10; i++) {
		TEST_ASSERT_EQUAL_UINT16(i, queuePeek(q));
		Idx item = queuePop(q);
		TEST_ASSERT_EQUAL_UINT16(i, item);
		TEST_ASSERT_EQUAL_UINT16(10 - i, queueSize(q));
	}

	TEST_ASSERT_TRUE(queueIsEmpty(q));
}

void testFIFOOrder(void)
{
	struct Queue *q = newQueue();

	for (Idx i = 100; i < 200; i++) {
		TEST_ASSERT_TRUE(queuePush(q, i));
	}

	for (Idx i = 100; i < 200; i++) {
		Idx item = queuePop(q);
		TEST_ASSERT_EQUAL_UINT16(i, item);
	}
}

void testFillQueue(void)
{
	struct Queue *q = newQueue();

	for (Idx i = 1; i < QUEUE_SIZE; i++) {
		TEST_ASSERT_FALSE(queueIsFull(q));
		TEST_ASSERT_TRUE(queuePush(q, i));
	}

	TEST_ASSERT_TRUE(queueIsFull(q));
	TEST_ASSERT_EQUAL_UINT16(QUEUE_SIZE - 1, queueSize(q));
}

void testWrapAround(void)
{
	struct Queue *q = newQueue();

	for (Idx i = 1; i <= QUEUE_SIZE / 2; i++) {
		TEST_ASSERT_TRUE(queuePush(q, i));
	}

	for (Idx i = 1; i <= QUEUE_SIZE / 4; i++) {
		Idx item = queuePop(q);
		TEST_ASSERT_EQUAL_UINT16(i, item);
	}

	for (Idx i = QUEUE_SIZE / 2 + 1; i < QUEUE_SIZE; i++) {
		TEST_ASSERT_TRUE(queuePush(q, i));
	}

	for (Idx i = QUEUE_SIZE / 4 + 1; i < QUEUE_SIZE; i++) {
		Idx item = queuePop(q);
		TEST_ASSERT_EQUAL_UINT16(i, item);
	}

	TEST_ASSERT_TRUE(queueIsEmpty(q));
}

void testPeekDoesNotModify(void)
{
	struct Queue *q = newQueue();

	TEST_ASSERT_TRUE(queuePush(q, 123));
	TEST_ASSERT_TRUE(queuePush(q, 456));

	TEST_ASSERT_EQUAL_UINT16(123, queuePeek(q));
	TEST_ASSERT_EQUAL_UINT16(123, queuePeek(q));
	TEST_ASSERT_EQUAL_UINT16(2, queueSize(q));

	queuePop(q);
	TEST_ASSERT_EQUAL_UINT16(456, queuePeek(q));
	TEST_ASSERT_EQUAL_UINT16(1, queueSize(q));
}

void testIteratorEmpty(void)
{
	struct Queue *q = newQueue();

	struct QueueIterator iter = queueIterate(q);
	Idx value = 0;
	TEST_ASSERT_FALSE(queueIteratorNext(&iter, &value));
	TEST_ASSERT_EQUAL_UINT16(0, value);
}

void testIteratorSingle(void)
{
	struct Queue *q = newQueue();
	TEST_ASSERT_TRUE(queuePush(q, 42));

	struct QueueIterator iter = queueIterate(q);
	Idx value = 0;

	TEST_ASSERT_TRUE(queueIteratorNext(&iter, &value));
	TEST_ASSERT_EQUAL_UINT16(42, value);

	TEST_ASSERT_FALSE(queueIteratorNext(&iter, &value));
}

void testIteratorMultiple(void)
{
	struct Queue *q = newQueue();

	for (Idx i = 10; i < 20; i++) {
		TEST_ASSERT_TRUE(queuePush(q, i));
	}

	struct QueueIterator iter = queueIterate(q);
	Idx value = 0;

	for (Idx i = 10; i < 20; i++) {
		TEST_ASSERT_TRUE(queueIteratorNext(&iter, &value));
		TEST_ASSERT_EQUAL_UINT16(i, value);
	}

	TEST_ASSERT_FALSE(queueIteratorNext(&iter, &value));
}

void testIteratorDoesNotModifyQueue(void)
{
	struct Queue *q = newQueue();

	for (Idx i = 1; i <= 5; i++) {
		TEST_ASSERT_TRUE(queuePush(q, i));
	}

	QueueIdx originalSize = queueSize(q);
	QueueIdx originalFront = q->front;
	QueueIdx originalBack = q->back;

	struct QueueIterator iter = queueIterate(q);
	Idx value = 0;
	while (queueIteratorNext(&iter, &value)) {
	}

	TEST_ASSERT_EQUAL_UINT16(originalSize, queueSize(q));
	TEST_ASSERT_EQUAL_UINT16(originalFront, q->front);
	TEST_ASSERT_EQUAL_UINT16(originalBack, q->back);
}

void testIteratorWithWrapAround(void)
{
	struct Queue *q = newQueue();

	for (Idx i = 1; i <= QUEUE_SIZE / 2; i++) {
		TEST_ASSERT_TRUE(queuePush(q, i));
	}

	for (Idx i = 1; i <= QUEUE_SIZE / 4; i++) {
		queuePop(q);
	}

	for (Idx i = QUEUE_SIZE / 2 + 1; i <= QUEUE_SIZE / 2 + 10; i++) {
		TEST_ASSERT_TRUE(queuePush(q, i));
	}

	struct QueueIterator iter = queueIterate(q);
	Idx value = 0;

	for (Idx i = QUEUE_SIZE / 4 + 1; i <= QUEUE_SIZE / 2 + 10; i++) {
		TEST_ASSERT_TRUE(queueIteratorNext(&iter, &value));
		TEST_ASSERT_EQUAL_UINT16(i, value);
	}

	TEST_ASSERT_FALSE(queueIteratorNext(&iter, &value));
}

void testPushPopCycles(void)
{
	struct Queue *q = newQueue();

	for (Idx cycle = 0; cycle < 10; cycle++) {
		for (Idx i = 1; i <= 50; i++) {
			TEST_ASSERT_TRUE(queuePush(q, cycle * 1000 + i));
		}

		TEST_ASSERT_EQUAL_UINT16(50, queueSize(q));

		for (Idx i = 1; i <= 50; i++) {
			Idx item = queuePop(q);
			TEST_ASSERT_EQUAL_UINT16(cycle * 1000 + i, item);
		}

		TEST_ASSERT_TRUE(queueIsEmpty(q));
	}
}

void testInsertWhenFull(void)
{
	struct Queue *q = newQueue();

	for (Idx i = 0; i < QUEUE_SIZE - 1; i++) {
		TEST_ASSERT_TRUE(queuePush(q, i));
	}

	TEST_ASSERT_EQUAL(QUEUE_SIZE - 1, queueSize(q));
	for (Idx i = 0; i < QUEUE_SIZE - 1; i++) {
		TEST_ASSERT_FALSE(queuePush(q, i));
	}
}

void testPushEmpty(void)
{
	struct Queue *q = newQueue();
	TEST_ASSERT_EQUAL_UINT16(-1, queuePop(q));

	for (Idx loops = 0; loops < 10; loops++) {
		for (Idx i = 0; i < QUEUE_SIZE - 1; i++) {
			TEST_ASSERT_TRUE(queuePush(q, i));
		}

		for (Idx i = 0; i < QUEUE_SIZE - 1; i++) {
			TEST_ASSERT_EQUAL_UINT16(i, queuePop(q));
		}

		for (Idx i = 0; i < QUEUE_SIZE - 1; i++) {
			TEST_ASSERT_EQUAL_UINT16(-1, queuePop(q));
		}
	}
}

void testPeekEmpty(void)
{
	struct Queue *q = newQueue();
	TEST_ASSERT_EQUAL_UINT16(-1, queuePeek(q));

	for (Idx loops = 0; loops < 10; loops++) {
		for (Idx i = 0; i < QUEUE_SIZE - 1; i++) {
			TEST_ASSERT_TRUE(queuePush(q, i));
		}

		for (Idx i = 0; i < QUEUE_SIZE - 1; i++) {
			TEST_ASSERT_EQUAL_UINT16(i, queuePop(q));
		}

		for (Idx i = 0; i < QUEUE_SIZE - 1; i++) {
			TEST_ASSERT_EQUAL_UINT16(-1, queuePeek(q));
		}
	}
}

void testSizeAccuracy(void)
{
	struct Queue *q = newQueue();

	TEST_ASSERT_EQUAL_UINT16(0, queueSize(q));

	for (Idx i = 0; i < QUEUE_SIZE - 1; i++) {
		TEST_ASSERT_TRUE(queuePush(q, i));
		TEST_ASSERT_EQUAL_UINT16(i + 1, queueSize(q));
	}

	for (Idx i = 0; i < QUEUE_SIZE - 1; i++) {
		queuePop(q);
		TEST_ASSERT_EQUAL_UINT16(QUEUE_SIZE - 2 - i, queueSize(q));
	}

	TEST_ASSERT_EQUAL_UINT16(0, queueSize(q));
}

void testAlternatingPushPop(void)
{
	struct Queue *q = newQueue();

	for (Idx i = 1; i <= 100; i++) {
		TEST_ASSERT_TRUE(queuePush(q, i));
		TEST_ASSERT_EQUAL_UINT16(1, queueSize(q));
		
		Idx item = queuePop(q);
		TEST_ASSERT_EQUAL_UINT16(i, item);
		TEST_ASSERT_EQUAL_UINT16(0, queueSize(q));
		TEST_ASSERT_TRUE(queueIsEmpty(q));
	}
}

int main(void)
{
	UNITY_BEGIN();

	/* Initialization */
	RUN_TEST(testInit);
	RUN_TEST(testInitDirty);

	/* Basic operations */
	RUN_TEST(testPushPop);
	RUN_TEST(testPushMultiple);
	RUN_TEST(testFIFOOrder);
	RUN_TEST(testPeekDoesNotModify);

	/* Capacity */
	RUN_TEST(testFillQueue);
	RUN_TEST(testWrapAround);
	RUN_TEST(testPushEmpty);
	RUN_TEST(testPeekEmpty);
	RUN_TEST(testSizeAccuracy);
	RUN_TEST(testInsertWhenFull);

	/* Iterator */
	RUN_TEST(testIteratorEmpty);
	RUN_TEST(testIteratorSingle);
	RUN_TEST(testIteratorMultiple);
	RUN_TEST(testIteratorDoesNotModifyQueue);
	RUN_TEST(testIteratorWithWrapAround);

	/* Stress tests */
	RUN_TEST(testPushPopCycles);
	RUN_TEST(testAlternatingPushPop);

	return UNITY_END();
}
