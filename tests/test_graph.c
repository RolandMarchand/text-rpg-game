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
	obstack_free(&arena, nullptr);
}

struct Graph *newGraph()
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
			       sizeof(g->nodes.head));

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

void testInit()
{
	struct Graph *g = ALLOC(sizeof(struct Graph));
	TEST_ASSERT_NOT_NULL(g);
	memset(g, 0, sizeof(struct Graph));
	testInitInternal(g);
}

void testInitDirty()
{
	struct Graph *g = ALLOC(sizeof(struct Graph));
	TEST_ASSERT_NOT_NULL(g);
	memset(g, 0xBB, sizeof(struct Graph));
	testInitInternal(g);
}

void testInsertEdgeList()
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

void testInsertEdgeHubOut()
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

void testInsertEdgeHubIn()
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

void testInsertEdgeLoop()
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

void testInsertEdgeFull()
{
	struct Graph *g = newGraph();

	for (GraphEdgeIdx i = 1; i < GRAPH_SIZE; i++) {
		TEST_ASSERT_TRUE(graphInsertEdge(g, i, i));
	}

	for (GraphEdgeIdx i = 1; i < GRAPH_SIZE; i++) {
		TEST_ASSERT_FALSE(graphInsertEdge(g, i, i));
	}
}

void testIteration()
{
	struct Graph *g = newGraph();

	for (GraphEdgeIdx i = 2; i < GRAPH_SIZE; i++) {
		TEST_ASSERT_TRUE(graphInsertEdge(g, 1, i));
	}

	struct GraphIterator iter = graphGetNeighbors(g, 1);
	for (GraphEdgeIdx i = 2; i < GRAPH_SIZE; i++) {
		GraphNodeIdx next = 0;
		TEST_ASSERT_TRUE(graphIteratorNext(&iter, &next));
		TEST_ASSERT_EQUAL_UINT(i, next);
	}

	for (GraphEdgeIdx i = 2; i < GRAPH_SIZE; i++) {
		GraphNodeIdx next = 0;
		TEST_ASSERT_FALSE(graphIteratorNext(&iter, &next));
		TEST_ASSERT_EQUAL_UINT(0, next);
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

	/* TODO:
	 * 1. delete edges sequentially first to last
	 * 2. delete edges sequentially last to first
	 * 3. delete edges in the middle
	 * 4. delete edges hub in
	 * 5. delete node hub out
	 * 6. delete node hub in
	 * 7. delete node no connections
	 * 8. delete node with only connections to itself
	 * 9. delete node with circular graph
	 * 10. delete and add edges randomly in succession
	 * 11. have a hub in, hub out, sequential, and test to detect edges
	 * 11. find shortest paths on grid
	 * 12. find shortest path on two disconnected nodes
	 * 13. find shortest path in loop
	 * 14. find shortest path on sequential
	 * 15. iterate over sequence that has been deleted in the middle */

	UNITY_END();
}
