#pragma once

#include <stdint.h>

#include "common.h"

#define GRAPH_SIZE MAX_DEFAULT

typedef Idx GraphEdgeIdx;
typedef Idx GraphNodeIdx;

struct Graph {
	struct {
		GraphEdgeIdx head[GRAPH_SIZE];
	} nodes;

	struct {
		GraphNodeIdx target[GRAPH_SIZE];
		GraphEdgeIdx nextEdge[GRAPH_SIZE];
		GraphEdgeIdx freeListHead;
		Idx count;
	} edges;
};

struct GraphIterator {
	const struct Graph *graph;
	GraphEdgeIdx currentEdge;
};

void graphInit(struct Graph *graph);
/* Return boolean indicating if inserting was successful. If from/to are out of bounds (<= 0, >= GRAPH_SIZE), or if graph is null, undefined behavior */
bool graphInsertEdge(struct Graph *graph, GraphNodeIdx from, GraphNodeIdx to);
/* Return whether it was found and deleted */
bool graphDeleteEdge(struct Graph *graph, GraphNodeIdx from, GraphNodeIdx to);
void graphDeleteNode(struct Graph *graph, GraphNodeIdx node);
bool graphHasEdge(const struct Graph *graph, GraphNodeIdx from,
		  GraphNodeIdx to);
struct GraphIterator graphGetNeighbors(const struct Graph *graph,
				       GraphNodeIdx node);
bool graphIteratorNext(struct GraphIterator *iter, GraphNodeIdx *out);

/* Path finding */
bool graphShortestPath(const struct Graph *graph, GraphNodeIdx start,
		       GraphNodeIdx goal, GraphNodeIdx outPath[GRAPH_SIZE],
		       int *outPathSize);
