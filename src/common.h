#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "laz_utils.h"
#include "const.h"
#include "errors.h"
#include "config.h"

#include "obstack/obstack.h"
#define obstack_chunk_alloc malloc
#define obstack_chunk_free free

typedef u16 Idx;
typedef Error(*Step)(void);

void exitGameLoop(void);
