#pragma once

#include "laz_utils.h"
#include "graph.h"

#define MAX_ROOMS 128

struct Rooms {
	char *names[MAX_ROOMS];
	char *descriptions[MAX_ROOMS];
	u16 depth[MAX_ROOMS];
	struct Graph layout;
};
