#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "graph.h"
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

struct Graph *newGraph(void)
{
	struct Graph *g = ALLOC(sizeof(struct Graph));
	TEST_ASSERT_NOT_NULL(g);
	graphInit(g);
	return g;
}

void testInitInternal(struct Graph *g)
{
	graphInit(g);

	GraphEdgeIdx *buf = ALLOC(GRAPH_SIZE * sizeof(GraphEdgeIdx));
	TEST_ASSERT_NOT_NULL(buf);
	memset(buf, 0, GRAPH_SIZE * sizeof(GraphEdgeIdx));

	_Static_assert(sizeof(g->nodes.head) == sizeof(g->edges.target) &&
		       sizeof(g->edges.target) == sizeof(g->edges.nextEdge) &&
		       GRAPH_SIZE * sizeof(GraphEdgeIdx) ==
		       sizeof(g->nodes.head), "Inconsistent types in struct Graph");

	TEST_ASSERT_EQUAL_MEMORY(buf, g->nodes.head, sizeof(g->nodes.head));
	TEST_ASSERT_EQUAL_MEMORY(buf, g->edges.target, sizeof(g->edges.target));

	size_t limit = sizeof(g->nodes.head) / sizeof(GraphEdgeIdx) - 1;
	for (size_t i = 1; i < limit; i++) {
		TEST_ASSERT_EQUAL_UINT(i + 1, g->edges.nextEdge[i]);
	}
	TEST_ASSERT_EQUAL_UINT(0, g->edges.nextEdge[0]);
	TEST_ASSERT_EQUAL_UINT(0, g->edges.nextEdge[limit]);

	TEST_ASSERT_EQUAL_UINT(1, g->edges.freeListHead);
	TEST_ASSERT_EQUAL_UINT16(0, g->edges.count);
}

void testInit(void)
{
	struct Graph *g = ALLOC(sizeof(struct Graph));
	TEST_ASSERT_NOT_NULL(g);
	memset(g, 0, sizeof(struct Graph));
	testInitInternal(g);
}

void testInitDirty(void)
{
	struct Graph *g = ALLOC(sizeof(struct Graph));
	TEST_ASSERT_NOT_NULL(g);
	memset(g, 0xBB, sizeof(struct Graph));
	testInitInternal(g);
}

void testInsertEdgeList(void)
{
	struct Graph *g = newGraph();

	for (GraphEdgeIdx i = 1; i < GRAPH_SIZE - 1; i++) {
		GraphEdgeIdx expectedEdge = g->edges.freeListHead;

		TEST_ASSERT_TRUE(graphInsertEdge(g, i, i + 1));

		TEST_ASSERT_EQUAL_UINT(expectedEdge, g->nodes.head[i]);
		TEST_ASSERT_EQUAL_UINT(i, g->edges.count);
		TEST_ASSERT_EQUAL_UINT(i + 1, g->edges.target[i]);
		TEST_ASSERT_EQUAL_UINT(0, g->edges.nextEdge[i]);
	}
}

void testInsertEdgeHubOut(void)
{
	struct Graph *g = newGraph();

	for (GraphEdgeIdx i = 2; i < GRAPH_SIZE; i++) {
		GraphEdgeIdx expectedEdge = g->edges.freeListHead;
		GraphEdgeIdx prevHead = g->nodes.head[1];

		TEST_ASSERT_TRUE(graphInsertEdge(g, 1, i));

		TEST_ASSERT_EQUAL_UINT(expectedEdge, g->nodes.head[1]);
		TEST_ASSERT_EQUAL_UINT(i - 1, g->edges.count);
		TEST_ASSERT_EQUAL_UINT(i, g->edges.target[expectedEdge]);
		TEST_ASSERT_EQUAL_UINT(prevHead,
				       g->edges.nextEdge[expectedEdge]);
	}
}

void testInsertEdgeHubIn(void)
{
	struct Graph *g = newGraph();

	for (GraphEdgeIdx i = 2; i < GRAPH_SIZE; i++) {
		GraphEdgeIdx expectedEdge = g->edges.freeListHead;
		GraphEdgeIdx prevHead = g->nodes.head[1];

		TEST_ASSERT_TRUE(graphInsertEdge(g, i, 1));

		TEST_ASSERT_EQUAL_UINT(expectedEdge, g->nodes.head[i]);
		TEST_ASSERT_EQUAL_UINT(i - 1, g->edges.count);
		TEST_ASSERT_EQUAL_UINT(1, g->edges.target[expectedEdge]);
		TEST_ASSERT_EQUAL_UINT(prevHead,
				       g->edges.nextEdge[expectedEdge]);
	}
}

void testInsertEdgeLoop(void)
{
	struct Graph *g = newGraph();

	for (GraphEdgeIdx i = 2; i < GRAPH_SIZE; i++) {
		GraphEdgeIdx expectedEdge = g->edges.freeListHead;
		GraphEdgeIdx prevHead = g->nodes.head[1];

		TEST_ASSERT_TRUE(graphInsertEdge(g, 1, 1));

		TEST_ASSERT_EQUAL_UINT(expectedEdge, g->nodes.head[1]);
		TEST_ASSERT_EQUAL_UINT(i - 1, g->edges.count);
		TEST_ASSERT_EQUAL_UINT(1, g->edges.target[expectedEdge]);
		TEST_ASSERT_EQUAL_UINT(prevHead,
				       g->edges.nextEdge[expectedEdge]);
	}
}

void testInsertEdgeFull(void)
{
	struct Graph *g = newGraph();

	for (GraphEdgeIdx i = 1; i < GRAPH_SIZE; i++) {
		TEST_ASSERT_TRUE(graphInsertEdge(g, i, i));
		TEST_ASSERT_EQUAL_UINT(i, g->edges.count);
	}

	for (GraphEdgeIdx i = 1; i < GRAPH_SIZE; i++) {
		TEST_ASSERT_FALSE(graphInsertEdge(g, i, i));
		TEST_ASSERT_EQUAL_UINT(GRAPH_SIZE - 1, g->edges.count);
	}
}

void testIteration(void)
{
	struct Graph *g = newGraph();

	for (GraphEdgeIdx i = 2; i < GRAPH_SIZE; i++) {
		TEST_ASSERT_TRUE(graphInsertEdge(g, 1, i));
	}

	struct GraphIterator iter = graphGetNeighbors(g, 1);
	for (GraphEdgeIdx i = GRAPH_SIZE - 1; i > 1; i--) {
		GraphNodeIdx next = 0;
		TEST_ASSERT_TRUE(graphIteratorNext(&iter, &next));
		TEST_ASSERT_EQUAL_UINT(i, next);
	}

	for (GraphEdgeIdx i = GRAPH_SIZE - 1; i > 1; i--) {
		GraphNodeIdx next = 0;
		TEST_ASSERT_FALSE(graphIteratorNext(&iter, &next));
		TEST_ASSERT_EQUAL_UINT(0, next);
	}
}

void testIterationOnDeleted(void)
{
	struct Graph *g = newGraph();

	TEST_ASSERT_TRUE(graphInsertEdge(g, 1, 2));
	TEST_ASSERT_TRUE(graphInsertEdge(g, 1, 3));
	TEST_ASSERT_TRUE(graphInsertEdge(g, 1, 4));
	TEST_ASSERT_TRUE(graphDeleteEdge(g, 1, 3));
	TEST_ASSERT_TRUE(graphInsertEdge(g, 1, 5));

	struct GraphIterator iter = graphGetNeighbors(g, 1);
	GraphNodeIdx next = 0;

	TEST_ASSERT_TRUE(graphIteratorNext(&iter, &next));
	TEST_ASSERT_EQUAL_UINT(5, next);
	TEST_ASSERT_TRUE(graphIteratorNext(&iter, &next));
	TEST_ASSERT_EQUAL_UINT(4, next);
	TEST_ASSERT_TRUE(graphIteratorNext(&iter, &next));
	TEST_ASSERT_EQUAL_UINT(2, next);
	TEST_ASSERT_FALSE(graphIteratorNext(&iter, &next));
}

void testIterationWhileDeleting(void)
{
	struct Graph *g = newGraph();

	for (int i = 2; i < GRAPH_SIZE; i++) {
		TEST_ASSERT_TRUE(graphInsertEdge(g, 1, i));
	}

	struct GraphIterator iter = graphGetNeighbors(g, 1);
	GraphNodeIdx next = 0;

	for (int i = GRAPH_SIZE - 1; i > 1; i--) {
		TEST_ASSERT_TRUE(graphIteratorNext(&iter, &next));
		TEST_ASSERT_EQUAL_UINT(i, next);
		TEST_ASSERT_TRUE(graphDeleteEdge(g, 1, i));
	}

	TEST_ASSERT_FALSE(graphIteratorNext(&iter, &next));
}

void testDetect(void)
{
	struct Graph *g = newGraph();

	for (GraphEdgeIdx i = 2; i < GRAPH_SIZE; i++) {
		TEST_ASSERT_TRUE(graphInsertEdge(g, 1, i));
		TEST_ASSERT_TRUE(graphHasEdge(g, 1, i));
	}

	for (GraphEdgeIdx i = 2; i < GRAPH_SIZE; i++) {
		TEST_ASSERT_TRUE(graphHasEdge(g, 1, i));
	}
}

void testDetectAfterDelete(void)
{
	struct Graph *g = newGraph();

	for (int i = 0; i < 10; i++) {
		for (GraphEdgeIdx i = 2; i < GRAPH_SIZE; i++) {
			TEST_ASSERT_TRUE(graphInsertEdge(g, 1, i));
			TEST_ASSERT_TRUE(graphHasEdge(g, 1, i));
			TEST_ASSERT_EQUAL_UINT(i - 1, g->edges.count);
		}

		for (GraphEdgeIdx i = 2; i < GRAPH_SIZE; i++) {
			TEST_ASSERT_TRUE(graphDeleteEdge(g, 1, i));
			TEST_ASSERT_FALSE(graphHasEdge(g, 1, i));
			TEST_ASSERT_EQUAL_UINT(GRAPH_SIZE - 1 - i,
					       g->edges.count);
		}

		for (GraphEdgeIdx i = 2; i < GRAPH_SIZE; i++) {
			TEST_ASSERT_FALSE(graphHasEdge(g, 1, i));
		}
	}
}

void testDeleteNoEdge(void)
{
	struct Graph *g = newGraph();

	GraphEdgeIdx expectedFreeHead = g->edges.freeListHead;
	TEST_ASSERT_EQUAL_UINT(0, g->edges.count);

	for (int i = 1; i < GRAPH_SIZE; i++) {
		for (int j = 1; j < GRAPH_SIZE; j++) {
			TEST_ASSERT_FALSE(graphDeleteEdge(g, i, j));
			TEST_ASSERT_EQUAL_UINT(0, g->edges.count);
			TEST_ASSERT_EQUAL(expectedFreeHead,
					  g->edges.freeListHead);
		}
	}
}

void testInsertAndDelete(void)
{
	struct Graph *g = newGraph();
	srand(0xDEADBEEF);

	Idx expectedEdgeCount = 0;
	char message[256] = { 0 };
	for (int i = 0; i < 50 * GRAPH_SIZE; i++) {
		TEST_ASSERT_LESS_THAN_UINT(GRAPH_SIZE, expectedEdgeCount);

		/* Promote more deletion */
		if (i % (GRAPH_SIZE * 3) == 0) {
			srand(0xDEADBEEF);
		}

		GraphNodeIdx from = rand() % (GRAPH_SIZE - 1) + 1;
		GraphNodeIdx to = rand() % (GRAPH_SIZE - 1) + 1;

		if (!graphHasEdge(g, from, to)) { /* Insert */
			if (g->edges.count >= 1023) {
				continue;
			}

			TEST_ASSERT_EQUAL_UINT(expectedEdgeCount,
					       g->edges.count);
			TEST_ASSERT_FALSE(graphDeleteEdge(g, from, to));

			TEST_ASSERT_EQUAL_UINT(expectedEdgeCount,
					       g->edges.count);

			TEST_ASSERT_LESS_OR_EQUAL(
				256,
				snprintf(
					message, 256,
					"iteration: %d, from: %d, to: %d, count: %d\n",
					i, from, to, g->edges.count));
			TEST_ASSERT_TRUE_MESSAGE(graphInsertEdge(g, from, to),
						 message);

			expectedEdgeCount++;
			TEST_ASSERT_EQUAL_UINT(expectedEdgeCount,
					       g->edges.count);
		} else { /* Delete */
			TEST_ASSERT_EQUAL_UINT(expectedEdgeCount,
					       g->edges.count);

			TEST_ASSERT_TRUE(graphDeleteEdge(g, from, to));

			expectedEdgeCount--;
			TEST_ASSERT_EQUAL_UINT(expectedEdgeCount,
					       g->edges.count);
		}
	}
}

int main(void)
{
	UNITY_BEGIN();

	/* Initialization */
	RUN_TEST(testInit);
	RUN_TEST(testInitDirty);

	/* Insertion on empty */
	RUN_TEST(testInsertEdgeList);
	RUN_TEST(testInsertEdgeHubOut);
	RUN_TEST(testInsertEdgeHubIn);
	RUN_TEST(testInsertEdgeLoop);
	RUN_TEST(testInsertEdgeFull);

	/* Iteration */
	RUN_TEST(testIteration);
	RUN_TEST(testIterationOnDeleted);
	RUN_TEST(testIterationWhileDeleting);

	/* Detect edges */
	RUN_TEST(testDetect);
	RUN_TEST(testDetectAfterDelete);

	/* Deletion */
	RUN_TEST(testDeleteNoEdge);
	RUN_TEST(testInsertAndDelete);

	/* TODO:
	 * 10. delete and add edges randomly in succession
	 * 11. find shortest paths on grid
	 * 12. find shortest path on two disconnected nodes
	 * 13. find shortest path in loop
	 * 14. find shortest path on sequential */

	return UNITY_END();
}
