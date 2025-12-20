#pragma once

#include "common.h"
#include "graph.h"

#define MAX_ROOMS 128

struct Rooms {
	char *names[MAX_ROOMS];
	char *descriptions[MAX_ROOMS];
	uint16_t depth[MAX_ROOMS];
	struct Graph layout;
};
