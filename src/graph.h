#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "common.h"

#define GRAPH_SIZE MAX_DEFAULT

typedef uint16_t EdgeIdx;
typedef uint16_t NodeIdx;

struct Graph {
	struct {
		EdgeIdx head[GRAPH_SIZE];
	} nodes;

	struct {
		NodeIdx target[GRAPH_SIZE];
		EdgeIdx nextEdge[GRAPH_SIZE];
	} edges;

	EdgeIdx freeListHead;
	uint16_t usedCount;
};

struct GraphIterator {
	struct Graph *graph;
	int currentEdge;
};

void graphInit(struct Graph *graph);
void graphInsertEdge(struct Graph *graph, NodeIdx from, NodeIdx to);
void graphDeleteNode(struct Graph *graph, NodeIdx node);
/* Return whether it was found */
bool graphDeleteEdge(struct Graph *graph, NodeIdx from, NodeIdx to);
bool graphHasEdge(struct Graph *graph, NodeIdx from, NodeIdx to);
struct GraphIterator graphGetNeighbors(struct Graph *graph, NodeIdx node);
bool graphIteratorNext(struct GraphIterator *iter, NodeIdx *out);
