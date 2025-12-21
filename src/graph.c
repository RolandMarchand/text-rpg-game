#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "graph.h"
#include "queue.h"

_Static_assert(1ull << sizeof(GraphNodeIdx) * CHAR_BIT >= GRAPH_SIZE);
_Static_assert(1ull << sizeof(GraphEdgeIdx) * CHAR_BIT >= GRAPH_SIZE);

void graphInit(struct Graph *graph)
{
	_Static_assert(1ull << sizeof(graph->edges.count) * CHAR_BIT >=
		       GRAPH_SIZE);

	assert(graph != nullptr);

	memset(graph, 0, sizeof(struct Graph));

	size_t limit = sizeof(graph->nodes.head) / sizeof(GraphEdgeIdx) - 1;
	for (size_t i = 1; i < limit; i++) {
		graph->edges.nextEdge[i] = i + 1;
	}

	graph->edges.freeListHead = 1;
}

bool graphInsertEdge(struct Graph *graph, GraphNodeIdx from, GraphNodeIdx to)
{
	assert(graph != nullptr);
	assert(from > 0 && from < GRAPH_SIZE);
	assert(to > 0 && to < GRAPH_SIZE);

	/* Indicates the graph is full */
	if (graph->edges.freeListHead == 0) {
		return false;
	}

	GraphEdgeIdx freeHead = graph->edges.freeListHead;
	graph->edges.freeListHead = graph->edges.nextEdge[freeHead];

	GraphEdgeIdx fromHead = graph->nodes.head[from];
	graph->nodes.head[from] = freeHead;
	graph->edges.target[freeHead] = to;
	graph->edges.nextEdge[freeHead] = fromHead;

	graph->edges.count += 1;

	return true;
}

void graphDeleteNode(struct Graph *graph, GraphNodeIdx node)
{
	assert(graph != nullptr);
	assert(node > 0 && node < GRAPH_SIZE);

	struct GraphIterator iter = graphGetNeighbors(graph, node);
	GraphNodeIdx linked = 0;
	while (graphIteratorNext(&iter, &linked)) {
		graphDeleteEdge(graph, node, linked);
	}

	for (GraphNodeIdx i = 1; i < GRAPH_SIZE; i++) {
		if (graph->nodes.head[i] == 0) {
			continue;
		}

		while (graphDeleteEdge(graph, i, node))
			;
	}

	graph->nodes.head[node] = 0;
}

bool graphDeleteEdge(struct Graph *graph, GraphNodeIdx from, GraphNodeIdx to)
{
	assert(graph != nullptr);
	assert(from > 0 && from < GRAPH_SIZE);
	assert(to > 0 && to < GRAPH_SIZE);

	GraphEdgeIdx edge = graph->nodes.head[from];
	GraphEdgeIdx start = edge;
	GraphEdgeIdx prev = 0;

	for (GraphNodeIdx i = 0; edge != 0 && i < GRAPH_SIZE; i++) {
		if (graph->edges.target[edge] != to) {
			assert(i == 0 || edge != start);
			prev = edge;
			edge = graph->edges.nextEdge[edge];
			continue;
		}

		if (prev == 0) {
			graph->nodes.head[from] = graph->edges.nextEdge[edge];
		} else {
			graph->edges.nextEdge[prev] =
				graph->edges.nextEdge[edge];
		}

		graph->edges.nextEdge[edge] = graph->edges.freeListHead;
		graph->edges.freeListHead = edge;
		graph->edges.count -= 1;
		return true;
	}

	return false;
}

bool graphHasEdge(const struct Graph *graph, GraphNodeIdx from, GraphNodeIdx to)
{
	assert(graph != nullptr);
	assert(from > 0 && from < GRAPH_SIZE);
	assert(to > 0 && to < GRAPH_SIZE);

	struct GraphIterator iter = graphGetNeighbors(graph, from);
	GraphNodeIdx linked = 0;
	while (graphIteratorNext(&iter, &linked)) {
		if (linked == to) {
			return true;
		}
	}

	return false;
}

struct GraphIterator graphGetNeighbors(const struct Graph *graph,
				       GraphNodeIdx node)
{
	assert(graph != nullptr);
	assert(node > 0 && node < GRAPH_SIZE);

	struct GraphIterator iter = { 0 };

	iter.graph = graph;
	iter.currentEdge = graph->nodes.head[node];

	return iter;
}

bool graphIteratorNext(struct GraphIterator *iter, GraphNodeIdx *out)
{
	assert(iter->graph != nullptr);
	assert(out != nullptr);

	if (iter->currentEdge == 0) {
		return false;
	}

	*out = iter->graph->edges.target[iter->currentEdge];
	iter->currentEdge = iter->graph->edges.nextEdge[iter->currentEdge];

	assert(*out != 0);

	return true;
}

static void graphVisitNeighbors(const struct Graph *graph, struct Queue *queue,
				GraphNodeIdx current,
				GraphNodeIdx parent[GRAPH_SIZE])
{
	struct GraphIterator iter = graphGetNeighbors(graph, current);
	GraphNodeIdx neighbor;
	while (graphIteratorNext(&iter, &neighbor)) {
		if (parent[neighbor] == 0) {
			parent[neighbor] = current;
			queuePush(queue, neighbor);
		}
	}
}

static void reversePath(GraphNodeIdx outPath[GRAPH_SIZE], int outPathSize)
{
	for (int i = 0; i < outPathSize / 2; i++) {
		GraphNodeIdx temp = outPath[i];
		outPath[i] = outPath[outPathSize - 1 - i];
		outPath[outPathSize - 1 - i] = temp;
	}
}

static int reconstructPath(GraphNodeIdx parent[GRAPH_SIZE], GraphNodeIdx start,
			   GraphNodeIdx goal, GraphNodeIdx outPath[GRAPH_SIZE])
{
	int size = 0;
	GraphNodeIdx node = goal;
	while (node != start && size < GRAPH_SIZE) {
		outPath[size] = node;
		size++;
		node = parent[node];
	}

	/* Bounds checking */
	assert(node == start);
	if (unlikely(node != start)) {
		return 0;
	}

	outPath[size] = start;
	size++;

	reversePath(outPath, size);

	return size;
}

bool graphShortestPath(const struct Graph *graph, GraphNodeIdx start,
		       GraphNodeIdx goal, GraphNodeIdx outPath[GRAPH_SIZE],
		       int *outPathSize)
{
	assert(graph != nullptr);
	assert(start > 0 && start < GRAPH_SIZE);
	assert(goal > 0 && goal < GRAPH_SIZE);
	assert(outPath != nullptr);
	assert(outPathSize != nullptr);

	static GraphNodeIdx parent[GRAPH_SIZE];
	memset(parent, 0, sizeof(parent));

	struct Queue queue;
	queueInit(&queue);

	parent[start] = start;
	queuePush(&queue, start);

	while (!queueIsEmpty(&queue)) {
		GraphNodeIdx current = queuePop(&queue);

		if (current != goal) {
			graphVisitNeighbors(graph, &queue, current, parent);
			continue;
		}

		*outPathSize = reconstructPath(parent, start, goal, outPath);

		return likely(*outPathSize > 0);
	}

	*outPathSize = 0;
	return false;
}
