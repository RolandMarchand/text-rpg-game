#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "graph.h"

void graphInit(struct Graph *graph)
{
	assert(graph != nullptr);

	/* Since graphs are allocated in global space, memory is already
	 * initialized to zero */

	size_t limit = sizeof(graph->nodes.head) / sizeof(EdgeIdx) - 1;
	for (size_t i = 1; i < limit; i++) {
		graph->edges.nextEdge[i] = i + 1;
	}

	graph->freeListHead = 1;
}

void graphInsertEdge(struct Graph *graph, NodeIdx from, NodeIdx to)
{
	assert(graph != nullptr);
	assert(from > 0 && from < GRAPH_SIZE);
	assert(to > 0 && to < GRAPH_SIZE);

	/* Indicates the graph is full */
	if (unlikely(graph->freeListHead == 0)) {
		assert(false);
		return;
	}

	EdgeIdx freeHead = graph->freeListHead;
	graph->freeListHead = graph->edges.nextEdge[freeHead];

	EdgeIdx fromHead = graph->nodes.head[from];
	graph->nodes.head[from] = freeHead;
	graph->edges.target[freeHead] = to;
	graph->edges.nextEdge[freeHead] = fromHead;

	graph->usedCount += 1;
}

void graphDeleteNode(struct Graph *graph, NodeIdx node)
{
	assert(graph != nullptr);
	assert(node > 0 && node < GRAPH_SIZE);

	struct GraphIterator iter = graphGetNeighbors(graph, node);
	NodeIdx linked = 0;
	while (graphIteratorNext(&iter, &linked)) {
		graphDeleteEdge(graph, node, linked);
	}

	for (NodeIdx i = 1; i < GRAPH_SIZE; i++) {
		if (graph->nodes.head[i] == 0) {
			continue;
		}

		while (graphDeleteEdge(graph, i, node));
	}

	graph->nodes.head[node] = 0;
}

bool graphDeleteEdge(struct Graph *graph, NodeIdx from, NodeIdx to)
{
	assert(graph != nullptr);
	assert(from > 0 && from < GRAPH_SIZE);
	assert(to > 0 && to < GRAPH_SIZE);

	EdgeIdx edge = graph->nodes.head[from];
	EdgeIdx start = edge;
	EdgeIdx prev = 0;

	for (NodeIdx i = 0; edge != 0 && i < GRAPH_SIZE; i++) {
		if (graph->edges.target[edge] != to) {
			assert(i == 0 || edge != start);
			prev = edge;
			edge = graph->edges.nextEdge[edge];
			continue;
		}

		if (prev == 0) {
			graph->nodes.head[from] = graph->edges.nextEdge[edge];
		} else {
			graph->edges.nextEdge[prev]
				= graph->edges.nextEdge[edge];
		}

		graph->edges.nextEdge[edge] = graph->freeListHead;
		graph->freeListHead = edge;
		graph->usedCount -= 1;
		return true;
	}

	return false;
}

bool graphHasEdge(struct Graph *graph, NodeIdx from, NodeIdx to)
{
	assert(graph != nullptr);
	assert(from > 0 && from < GRAPH_SIZE);
	assert(to > 0 && to < GRAPH_SIZE);

	struct GraphIterator iter = graphGetNeighbors(graph, from);
	NodeIdx linked = 0;
	while (graphIteratorNext(&iter, &linked)) {
		if (linked == to) {
			return true;
		}
	}

	return false;
}

struct GraphIterator graphGetNeighbors(struct Graph *graph, NodeIdx node)
{
	assert(graph != nullptr);
	assert(node > 0 && node < GRAPH_SIZE);

	struct GraphIterator iter = { 0 };

	iter.graph = graph;
	iter.currentEdge = graph->nodes.head[node];

	return iter;
}

bool graphIteratorNext(struct GraphIterator *iter, NodeIdx *out)
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
