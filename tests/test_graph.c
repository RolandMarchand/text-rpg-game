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
			       sizeof(g->edges.target) ==
				       sizeof(g->edges.nextEdge) &&
			       GRAPH_SIZE * sizeof(GraphEdgeIdx) ==
				       sizeof(g->nodes.head),
		       "Inconsistent types in struct Graph");

	TEST_ASSERT_EQUAL_MEMORY(buf, g->nodes.head, sizeof(g->nodes.head));
	TEST_ASSERT_EQUAL_MEMORY(buf, g->edges.target, sizeof(g->edges.target));

	size_t limit = sizeof(g->nodes.head) / sizeof(GraphEdgeIdx) - 1;
	for (size_t i = 1; i < limit; i++) {
		TEST_ASSERT_EQUAL_UINT16(i + 1, g->edges.nextEdge[i]);
	}
	TEST_ASSERT_EQUAL_UINT16(0, g->edges.nextEdge[0]);
	TEST_ASSERT_EQUAL_UINT16(0, g->edges.nextEdge[limit]);

	TEST_ASSERT_EQUAL_UINT16(1, g->edges.freeListHead);
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

		TEST_ASSERT_EQUAL_UINT16(expectedEdge, g->nodes.head[i]);
		TEST_ASSERT_EQUAL_UINT16(i, g->edges.count);
		TEST_ASSERT_EQUAL_UINT16(i + 1, g->edges.target[i]);
		TEST_ASSERT_EQUAL_UINT16(0, g->edges.nextEdge[i]);
	}
}

void testInsertEdgeHubOut(void)
{
	struct Graph *g = newGraph();

	for (GraphEdgeIdx i = 2; i < GRAPH_SIZE; i++) {
		GraphEdgeIdx expectedEdge = g->edges.freeListHead;
		GraphEdgeIdx prevHead = g->nodes.head[1];

		TEST_ASSERT_TRUE(graphInsertEdge(g, 1, i));

		TEST_ASSERT_EQUAL_UINT16(expectedEdge, g->nodes.head[1]);
		TEST_ASSERT_EQUAL_UINT16(i - 1, g->edges.count);
		TEST_ASSERT_EQUAL_UINT16(i, g->edges.target[expectedEdge]);
		TEST_ASSERT_EQUAL_UINT16(prevHead,
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

		TEST_ASSERT_EQUAL_UINT16(expectedEdge, g->nodes.head[i]);
		TEST_ASSERT_EQUAL_UINT16(i - 1, g->edges.count);
		TEST_ASSERT_EQUAL_UINT16(1, g->edges.target[expectedEdge]);
		TEST_ASSERT_EQUAL_UINT16(prevHead,
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

		TEST_ASSERT_EQUAL_UINT16(expectedEdge, g->nodes.head[1]);
		TEST_ASSERT_EQUAL_UINT16(i - 1, g->edges.count);
		TEST_ASSERT_EQUAL_UINT16(1, g->edges.target[expectedEdge]);
		TEST_ASSERT_EQUAL_UINT16(prevHead,
				       g->edges.nextEdge[expectedEdge]);
	}
}

void testInsertEdgeFull(void)
{
	struct Graph *g = newGraph();

	for (GraphEdgeIdx i = 1; i < GRAPH_SIZE; i++) {
		TEST_ASSERT_TRUE(graphInsertEdge(g, i, i));
		TEST_ASSERT_EQUAL_UINT16(i, g->edges.count);
	}

	for (GraphEdgeIdx i = 1; i < GRAPH_SIZE; i++) {
		TEST_ASSERT_FALSE(graphInsertEdge(g, i, i));
		TEST_ASSERT_EQUAL_UINT16(GRAPH_SIZE - 1, g->edges.count);
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
		TEST_ASSERT_EQUAL_UINT16(i, next);
	}

	for (GraphEdgeIdx i = GRAPH_SIZE - 1; i > 1; i--) {
		GraphNodeIdx next = 0;
		TEST_ASSERT_FALSE(graphIteratorNext(&iter, &next));
		TEST_ASSERT_EQUAL_UINT16(0, next);
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
	TEST_ASSERT_EQUAL_UINT16(5, next);
	TEST_ASSERT_TRUE(graphIteratorNext(&iter, &next));
	TEST_ASSERT_EQUAL_UINT16(4, next);
	TEST_ASSERT_TRUE(graphIteratorNext(&iter, &next));
	TEST_ASSERT_EQUAL_UINT16(2, next);
	TEST_ASSERT_FALSE(graphIteratorNext(&iter, &next));
}

void testIterationWhileDeleting(void)
{
	struct Graph *g = newGraph();

	for (GraphEdgeIdx i = 2; i < GRAPH_SIZE; i++) {
		TEST_ASSERT_TRUE(graphInsertEdge(g, 1, i));
	}

	struct GraphIterator iter = graphGetNeighbors(g, 1);
	GraphNodeIdx next = 0;

	for (GraphNodeIdx i = GRAPH_SIZE - 1; i > 1; i--) {
		TEST_ASSERT_TRUE(graphIteratorNext(&iter, &next));
		TEST_ASSERT_EQUAL_UINT16(i, next);
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

	for (GraphNodeIdx i = 0; i < 10; i++) {
		for (GraphEdgeIdx i = 2; i < GRAPH_SIZE; i++) {
			TEST_ASSERT_TRUE(graphInsertEdge(g, 1, i));
			TEST_ASSERT_TRUE(graphHasEdge(g, 1, i));
			TEST_ASSERT_EQUAL_UINT16(i - 1, g->edges.count);
		}

		for (GraphEdgeIdx i = 2; i < GRAPH_SIZE; i++) {
			TEST_ASSERT_TRUE(graphDeleteEdge(g, 1, i));
			TEST_ASSERT_FALSE(graphHasEdge(g, 1, i));
			TEST_ASSERT_EQUAL_UINT16(GRAPH_SIZE - 1 - i,
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
	TEST_ASSERT_EQUAL_UINT16(0, g->edges.count);

	for (GraphNodeIdx i = 1; i < GRAPH_SIZE; i++) {
		for (GraphNodeIdx j = 1; j < GRAPH_SIZE; j++) {
			TEST_ASSERT_FALSE(graphDeleteEdge(g, i, j));
			TEST_ASSERT_EQUAL_UINT16(0, g->edges.count);
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
		TEST_ASSERT_LESS_THAN_UINT16(GRAPH_SIZE, expectedEdgeCount);

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

			TEST_ASSERT_EQUAL_UINT16(expectedEdgeCount,
					       g->edges.count);
			TEST_ASSERT_FALSE(graphDeleteEdge(g, from, to));

			TEST_ASSERT_EQUAL_UINT16(expectedEdgeCount,
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
			TEST_ASSERT_EQUAL_UINT16(expectedEdgeCount,
					       g->edges.count);
		} else { /* Delete */
			TEST_ASSERT_EQUAL_UINT16(expectedEdgeCount,
					       g->edges.count);

			TEST_ASSERT_TRUE(graphDeleteEdge(g, from, to));

			expectedEdgeCount--;
			TEST_ASSERT_EQUAL_UINT16(expectedEdgeCount,
					       g->edges.count);
		}
	}
}

void testDeleteNode(void)
{
	struct Graph *g = newGraph();
	const GraphNodeIdx toDelete = 2;

	for (GraphNodeIdx i = 0; i < 100; i++) {
		TEST_ASSERT_TRUE(graphInsertEdge(g, 1, toDelete));
		TEST_ASSERT_TRUE(graphInsertEdge(g, toDelete, 1));
		TEST_ASSERT_EQUAL((i + 1) * 2, g->edges.count);
	}

	graphDeleteNode(g, toDelete);
	TEST_ASSERT_EQUAL(0, g->edges.count);
	TEST_ASSERT_EQUAL(0, g->nodes.head[1]);
	TEST_ASSERT_EQUAL(0, g->nodes.head[toDelete]);
}

void testDeleteNodeWithMultipleNeighbors(void)
{
	struct Graph *g = newGraph();
	const GraphNodeIdx toDelete = 50;

	for (GraphNodeIdx i = 1; i < 20; i++) {
		TEST_ASSERT_TRUE(graphInsertEdge(g, i, toDelete));
	}

	for (GraphNodeIdx i = 51; i < 70; i++) {
		TEST_ASSERT_TRUE(graphInsertEdge(g, toDelete, i));
	}

	int expectedCount = 19 + 19;
	TEST_ASSERT_EQUAL(expectedCount, g->edges.count);

	graphDeleteNode(g, toDelete);

	TEST_ASSERT_EQUAL(0, g->edges.count);
	TEST_ASSERT_EQUAL(0, g->nodes.head[toDelete]);

	for (GraphNodeIdx i = 1; i < 20; i++) {
		TEST_ASSERT_FALSE(graphHasEdge(g, i, toDelete));
	}
}

void testShortestPathLinear(void)
{
	struct Graph *g = newGraph();
	for (GraphNodeIdx i = 1; i < GRAPH_SIZE - 1; i++) {
		TEST_ASSERT_TRUE(graphInsertEdge(g, i, i + 1));
		GraphNodeIdx path[GRAPH_SIZE] = { 0 };
		int pathSize = 0;
		TEST_ASSERT_TRUE(
			graphShortestPath(g, 1, i + 1, path, &pathSize));
		TEST_ASSERT_EQUAL(i + 1, pathSize);
		for (GraphNodeIdx j = 0; j < pathSize; j++) {
			TEST_ASSERT_EQUAL(j + 1, path[j]);
		}
	}
}

void testShortestPathThroughHub(void)
{
	struct Graph *g = newGraph();
	const GraphNodeIdx start = 1;
	const GraphNodeIdx hub = 2;
	const GraphNodeIdx goal = 3;

	TEST_ASSERT_TRUE(graphInsertEdge(g, start, hub));
	TEST_ASSERT_TRUE(graphInsertEdge(g, hub, goal));

	for (GraphNodeIdx i = 3; i < GRAPH_SIZE - 3; i++) {
		TEST_ASSERT_TRUE(graphInsertEdge(g, hub, i));
	}

	GraphNodeIdx path[GRAPH_SIZE] = { 0 };
	int size = 0;

	TEST_ASSERT_TRUE(graphShortestPath(g, start, goal, path, &size));
	TEST_ASSERT_EQUAL(3, size);
	TEST_ASSERT_EQUAL(1, path[0]);
	TEST_ASSERT_EQUAL(2, path[1]);
	TEST_ASSERT_EQUAL(3, path[2]);
	TEST_ASSERT_EQUAL(0, path[3]);
}

void testShortestPathThroughHub2(void)
{
	struct Graph *g = newGraph();
	const GraphNodeIdx start = 1;
	const GraphNodeIdx goal = 200;
	_Static_assert(GRAPH_SIZE >= 100,
		       "GRAPH_SIZE needs to be >= than 100 for this test");

	TEST_ASSERT_TRUE(graphInsertEdge(g, start, 2));

	for (GraphNodeIdx i = 3; i < 20; i++) {
		TEST_ASSERT_TRUE(graphInsertEdge(g, 2, i));
	}

	for (GraphNodeIdx i = 20; i < 40; i++) {
		TEST_ASSERT_TRUE(graphInsertEdge(g, 3, i));
	}

	for (GraphNodeIdx i = 40; i < 60; i++) {
		TEST_ASSERT_TRUE(graphInsertEdge(g, 4, i));
	}

	for (GraphNodeIdx i = 60; i < 80; i++) {
		TEST_ASSERT_TRUE(graphInsertEdge(g, 21, i));
	}

	for (GraphNodeIdx i = 80; i < 100; i++) {
		TEST_ASSERT_TRUE(graphInsertEdge(g, 20, i));
	}

	TEST_ASSERT_TRUE(graphInsertEdge(g, 70, goal));

	GraphNodeIdx path[GRAPH_SIZE] = { 0 };
	int size = 0;

	TEST_ASSERT_TRUE(graphShortestPath(g, start, goal, path, &size));

	TEST_ASSERT_EQUAL(6, size);
	TEST_ASSERT_EQUAL(start, path[0]);
	TEST_ASSERT_EQUAL(2, path[1]);
	TEST_ASSERT_EQUAL(3, path[2]);
	TEST_ASSERT_EQUAL(21, path[3]);
	TEST_ASSERT_EQUAL(70, path[4]);
	TEST_ASSERT_EQUAL(goal, path[5]);
	TEST_ASSERT_EQUAL(0, path[6]);
}

void testNoShortestPath(void)
{
	struct Graph *g = newGraph();
	const GraphNodeIdx start = 1;
	const GraphNodeIdx goal = 200;
	_Static_assert(GRAPH_SIZE >= 100,
		       "GRAPH_SIZE needs to be >= than 100 for this test");

	TEST_ASSERT_TRUE(graphInsertEdge(g, start, 2));

	for (GraphNodeIdx i = 3; i < 20; i++) {
		TEST_ASSERT_TRUE(graphInsertEdge(g, 2, i));
	}

	for (GraphNodeIdx i = 20; i < 40; i++) {
		TEST_ASSERT_TRUE(graphInsertEdge(g, 3, i));
	}

	for (GraphNodeIdx i = 40; i < 60; i++) {
		TEST_ASSERT_TRUE(graphInsertEdge(g, 4, i));
	}

	for (GraphNodeIdx i = 60; i < 80; i++) {
		TEST_ASSERT_TRUE(graphInsertEdge(g, 21, i));
	}

	for (GraphNodeIdx i = 80; i < 100; i++) {
		TEST_ASSERT_TRUE(graphInsertEdge(g, 20, i));
	}

	/* TEST_ASSERT_TRUE(graphInsertEdge(g, 70, goal)); */

	GraphNodeIdx path[GRAPH_SIZE] = { 0 };
	int size = 0;

	TEST_ASSERT_FALSE(graphShortestPath(g, start, goal, path, &size));

	TEST_ASSERT_EQUAL(0, size);
	for (GraphNodeIdx i = 0; i < GRAPH_SIZE; i++) {
		TEST_ASSERT_EQUAL(0, path[i]);
	}
}

void testShortestPathFromTwo(void)
{
	struct Graph *g = newGraph();
	const GraphNodeIdx start = 1;
	const GraphNodeIdx goal = 200;

	for (GraphNodeIdx i = start; i < 100; i++) {
		TEST_ASSERT_TRUE(graphInsertEdge(g, i, i + 1));
	}
	TEST_ASSERT_TRUE(graphInsertEdge(g, start, 101));
	TEST_ASSERT_TRUE(graphInsertEdge(g, 2, 101));
	TEST_ASSERT_TRUE(graphInsertEdge(g, 101, goal));

	GraphNodeIdx path[GRAPH_SIZE] = { 0 };
	int size = 0;

	TEST_ASSERT_TRUE(graphShortestPath(g, start, goal, path, &size));

	TEST_ASSERT_EQUAL(3, size);
	TEST_ASSERT_EQUAL(start, path[0]);
	TEST_ASSERT_EQUAL(101, path[1]);
	TEST_ASSERT_EQUAL(goal, path[2]);
	TEST_ASSERT_EQUAL(0, path[3]);
}

void testShortestPathSingleNode(void)
{
	struct Graph *g = newGraph();

	GraphNodeIdx path[GRAPH_SIZE] = { 0 };
	int size = 0;

	TEST_ASSERT_TRUE(graphShortestPath(g, 1, 1, path, &size));
	TEST_ASSERT_EQUAL(1, size);
	TEST_ASSERT_EQUAL(1, path[0]);
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
	RUN_TEST(testDeleteNode);
	RUN_TEST(testDeleteNodeWithMultipleNeighbors);

	RUN_TEST(testShortestPathLinear);
	RUN_TEST(testShortestPathThroughHub);
	RUN_TEST(testShortestPathThroughHub2);
	RUN_TEST(testNoShortestPath);
	RUN_TEST(testShortestPathFromTwo);
	RUN_TEST(testShortestPathSingleNode);

	return UNITY_END();
}
