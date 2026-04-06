#pragma once

#include <stdlib.h>
#include "../errors.h"

#include "obstack.h"
#define obstack_chunk_alloc malloc
#define obstack_chunk_free free

extern struct obstack arena;

Error initArena(void);
Error cleanupArena(void);
char *duplicateString(const char *str);
