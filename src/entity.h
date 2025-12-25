#pragma once

#include <stdint.h>

#include "const.h"

#define MAX_ENTITIES MAX_DEFAULT

typedef uint16_t EntityIdx;
typedef uint8_t EntityType;

enum {
	TYPE_NONE = 0,
	TYPE_BEAST = 1 << 0,
	TYPE_UNDEAD = 1 << 1,
	TYPE_DEMON = 1 << 2,
	TYPE_ELEMENTAL = 1 << 3,
	TYPE_FUNGUS = 1 << 4,
	TYPE_HUMAN = 1 << 5,
	TYPE_KOBOLD = 1 << 6,
	TYPE_CELESTIAL = 1 << 7,
};

_Static_assert((TYPE_BEAST | TYPE_UNDEAD | TYPE_DEMON | TYPE_ELEMENTAL | 
                TYPE_FUNGUS | TYPE_HUMAN | TYPE_KOBOLD | TYPE_CELESTIAL) <= UINT8_MAX, 
               "EntityType flags exceed uint8_t range");

struct EntityComponents {
	/* Entity 0 is special, it's the null entity */
	char *names[MAX_ENTITIES];
	char *description[MAX_ENTITIES];
	float healths[MAX_ENTITIES];
	EntityIdx location[MAX_ENTITIES];
	EntityType types[MAX_ENTITIES];
};
