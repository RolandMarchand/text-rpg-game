#pragma once

#include <stdint.h>

#include "const.h"

#define MAX_ENTITIES MAX_DEFAULT

typedef uint16_t EntityIdx;

typedef enum Type: uint8_t {
	NONE = 0,
	BEAST = 1 << 0,
	UNDEAD = 1 << 1,
	DEMON = 1 << 2,
	ELEMENTAL = 1 << 3,
	FUNGUS = 1 << 4,
	HUMAN = 1 << 5,
	KOBOLD = 1 << 6,
	CELESTIAL = 1 << 7,
} Type;

struct EntityComponents {
	/* Entity 0 is special, it's the null entity */
	char *names[MAX_ENTITIES];
	char *description[MAX_ENTITIES];
	float healths[MAX_ENTITIES];
	EntityIdx location[MAX_ENTITIES];
	Type types[MAX_ENTITIES];
};
