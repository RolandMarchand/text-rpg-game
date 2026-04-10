#pragma once

#include <stdint.h>

#include "common.h"

#define MAX_ENTITIES MAX_DEFAULT

typedef u16 EntityIdx;
typedef u8 EntityType;

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

/* There should be some dude named Jerry. He kind of just talks to you. He went
 * to college with the player. They did sports ball together, and did some
 * naughty stuff in the players' bathroom. Only once, it was awkward and never
 * talked about again. You didn't forget, but you think Jerry might have. He did
 * not. Little did you know, Jerry broke up with all of his exes because
 * whenever he had sex with them, he could only think of you. You are okay with
 * this. Jerry is actually a cool dude, and you can see yourself raising
 * children together. Jerry is the right role model, and you are the wrong one.
 * Ying and yang. But unfortunately, despite destiny having woven your paths
 * together, you're just not over your ex, and your ex wasn't written in the
 * destiny so it kind of just sucks for you. While you talk to Jerry, you replay
 * that evening in the showers with him for the thousandth time. Each time was
 * better than the last. Jerry talks to you about his new hot sauce idea for the
 * salad bar. You don't like salad, but you'll try it anyway. For Jerry. Jerry's
 * sauce.
 *
 * — Haru */

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
